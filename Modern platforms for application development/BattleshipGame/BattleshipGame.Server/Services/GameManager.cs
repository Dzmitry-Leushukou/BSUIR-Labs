using BattleshipGame.Shared.Models;
using BattleshipGame.Shared.Validation;

namespace BattleshipGame.Server.Services
{
    public class GameManager
    {
        private readonly Dictionary<string, GameSession> _sessions = new();
        private readonly ILogger<GameManager> _logger;

        public GameManager(ILogger<GameManager> logger)
        {
            _logger = logger;
        }

        // Создание новой игры
        public GameSession CreateNewGame()
        {
            var sessionId = Guid.NewGuid().ToString("N")[..8];
            var session = new GameSession
            {
                SessionId = sessionId,
                CreatedAt = DateTime.UtcNow,
                State = GameState.WaitingForPlayers
            };

            _sessions[sessionId] = session;
            _logger.LogInformation("Создана новая игра: {SessionId}", sessionId);

            return session;
        }

        // Получение сессии по ID
        public GameSession? GetSession(string sessionId)
        {
            return _sessions.TryGetValue(sessionId, out var session) ? session : null;
        }

        // Добавление игрока в сессию
        public bool AddPlayerToSession(string sessionId, Player player)
        {
            var session = GetSession(sessionId);
            if (session == null)
            {
                _logger.LogWarning("Сессия не найдена: {SessionId}", sessionId);
                return false;
            }

            // Проверяем, не присоединился ли уже игрок
            if (session.Players.Any(p => p.ConnectionId == player.ConnectionId))
            {
                return true;
            }

            if (session.TryAddPlayer(player))
            {
                _logger.LogInformation("Игрок {PlayerName} присоединился к сессии {SessionId}",
                    player.Name, sessionId);
                return true;
            }

            _logger.LogWarning("Не удалось добавить игрока в сессию {SessionId}, игра уже полная", sessionId);
            return false;
        }

        // Размещение кораблей игрока
        public bool PlaceShips(string sessionId, string connectionId, List<Ship> ships)
        {
            var session = GetSession(sessionId);
            if (session == null) return false;

            var player = session.GetPlayer(connectionId);
            if (player == null) return false;

            // Валидация расстановки кораблей
            if (!ShipValidator.ValidateAllShips(ships))
            {
                _logger.LogWarning("Невалидная расстановка кораблей для игрока {PlayerName}", player.Name);
                return false;
            }

            player.Ships = ships;
            player.HasPlacedShips = true;

            _logger.LogInformation("Игрок {PlayerName} разместил корабли в сессии {SessionId}",
                player.Name, sessionId);

            return true;
        }

        // Обработка выстрела
        public ShotResult ProcessShot(string sessionId, string shooterConnectionId, int x, int y)
        {
            var session = GetSession(sessionId);
            if (session == null)
                return new ShotResult { IsValid = false, Error = "Сессия не найдена" };

            if (session.State != GameState.InProgress)
                return new ShotResult { IsValid = false, Error = "Игра не в процессе" };

            // Проверяем, ход ли стреляющего
            if (!session.IsPlayerTurn(shooterConnectionId))
                return new ShotResult { IsValid = false, Error = "Не ваш ход" };

            var shooter = session.GetPlayer(shooterConnectionId);
            var target = session.GetOpponent(shooterConnectionId);

            if (shooter == null || target == null)
                return new ShotResult { IsValid = false, Error = "Игрок не найден" };

            // Проверяем границы
            if (x < 0 || x >= 10 || y < 0 || y >= 10)
                return new ShotResult { IsValid = false, Error = "Выстрел за пределы поля" };

            // Проверяем, не стреляли ли уже в эту клетку
            var existingShot = shooter.ShotsFired.FirstOrDefault(s => s.X == x && s.Y == y);
            if (existingShot != null)
                return new ShotResult { IsValid = false, Error = "Уже стреляли в эту клетку" };

            // Создаем выстрел
            var shot = new Shot
            {
                X = x,
                Y = y,
                PlayerConnectionId = shooterConnectionId
            };

            // Проверяем попадание
            var hitShip = target.Ships.FirstOrDefault(ship =>
                ship.Cells.Any(cell => cell.X == x && cell.Y == y));

            if (hitShip != null)
            {
                // Попадание!
                shot.IsHit = true;
                var hitCell = hitShip.Cells.First(c => c.X == x && c.Y == y);
                hitCell.IsHit = true;

                // Добавляем выстрелы
                shooter.ShotsFired.Add(shot);
                target.ShotsReceived.Add(shot);

                // Проверяем, уничтожен ли корабль
                bool isShipDestroyed = hitShip.IsDestroyed;

                // Проверяем, окончена ли игра
                bool isGameOver = target.Ships.All(s => s.IsDestroyed);

                if (isGameOver)
                {
                    session.State = GameState.Finished;
                    session.WinnerConnectionId = shooterConnectionId;
                    _logger.LogInformation("Игра окончена в сессии {SessionId}. Победитель: {PlayerName}",
                        sessionId, shooter.Name);
                }

                _logger.LogInformation("Попадание в сессии {SessionId} по координатам ({X},{Y})",
                    sessionId, x, y);

                return new ShotResult
                {
                    IsValid = true,
                    IsHit = true,
                    IsShipDestroyed = isShipDestroyed,
                    IsGameOver = isGameOver,
                    ShipName = hitShip.Name
                    // При попадании ход остается у текущего игрока (не меняем ShooterKeepsTurn)
                };
            }
            else
            {
                // Промах
                shot.IsHit = false;
                shooter.ShotsFired.Add(shot);
                target.ShotsReceived.Add(shot);

                _logger.LogInformation("Промах в сессии {SessionId} по координатам ({X},{Y})",
                    sessionId, x, y);

                return new ShotResult
                {
                    IsValid = true,
                    IsHit = false,
                    IsGameOver = false
                    // При промахе ход переходит (ShooterKeepsTurn = false по умолчанию)
                };
            }
        }

        // Удаление игрока из сессии
        public void RemovePlayer(string connectionId)
        {
            foreach (var session in _sessions.Values)
            {
                var player = session.GetPlayer(connectionId);
                if (player != null)
                {
                    _logger.LogInformation("Игрок {PlayerName} покинул сессию {SessionId}",
                        player.Name, session.SessionId);

                    // Если игра в процессе, отмечаем победителем другого игрока
                    if (session.State == GameState.InProgress)
                    {
                        var opponent = session.GetOpponent(connectionId);
                        if (opponent != null)
                        {
                            session.State = GameState.Finished;
                            session.WinnerConnectionId = opponent.ConnectionId;
                            _logger.LogInformation("Игра в сессии {SessionId} завершена. Победитель: {PlayerName}",
                                session.SessionId, opponent.Name);
                        }
                    }

                    // Удаляем игрока из сессии
                    if (session.Player1?.ConnectionId == connectionId)
                        session.Player1 = null;
                    else if (session.Player2?.ConnectionId == connectionId)
                        session.Player2 = null;

                    // Если в сессии не осталось игроков, удаляем сессию
                    if (session.Player1 == null && session.Player2 == null)
                    {
                        _sessions.Remove(session.SessionId);
                        _logger.LogInformation("Сессия {SessionId} удалена", session.SessionId);
                    }

                    break;
                }
            }
        }

        // Получение активных сессий
        public List<GameSession> GetActiveSessions()
        {
            return _sessions.Values
                .Where(s => s.State != GameState.Finished)
                .ToList();
        }

        // Получение сессии по connectionId игрока
        public GameSession? GetSessionByPlayerConnection(string connectionId)
        {
            return _sessions.Values.FirstOrDefault(s =>
                s.Player1?.ConnectionId == connectionId ||
                s.Player2?.ConnectionId == connectionId);
        }
    }

    // Класс для результата выстрела
    public class ShotResult
    {
        public bool IsValid { get; set; }
        public string? Error { get; set; }
        public bool IsHit { get; set; }
        public bool IsShipDestroyed { get; set; }
        public bool IsGameOver { get; set; }
        public string? ShipName { get; set; }
    }
}