using Microsoft.AspNetCore.SignalR;
using BattleshipGame.Shared.Models;
using BattleshipGame.Shared.Hubs;
using BattleshipGame.Server.Services;

namespace BattleshipGame.Server.Hubs
{
    public class GameHub : Hub<IGameClient>
    {
        private readonly GameManager _gameManager;
        private readonly ILogger<GameHub> _logger;

        public GameHub(GameManager gameManager, ILogger<GameHub> logger)
        {
            _gameManager = gameManager;
            _logger = logger;
        }

        // Создание новой игры
        public async Task<string> CreateGame(string playerName)
        {
            var session = _gameManager.CreateNewGame();
            var player = new Player(Context.ConnectionId, playerName);

            if (_gameManager.AddPlayerToSession(session.SessionId, player))
            {
                await Groups.AddToGroupAsync(Context.ConnectionId, session.SessionId);
                await Clients.Caller.GameCreated(session.SessionId);
                await Clients.Caller.ShowMessage($"Игра создана! ID: {session.SessionId}");

                _logger.LogInformation("Игрок {PlayerName} создал игру {SessionId}",
                    playerName, session.SessionId);

                return session.SessionId;
            }

            await Clients.Caller.ShowError("Не удалось создать игру");
            throw new HubException("Не удалось создать игру");
        }

        // Присоединение к существующей игре
        public async Task<bool> JoinGame(string sessionId, string playerName)
        {
            var session = _gameManager.GetSession(sessionId);
            if (session == null)
            {
                await Clients.Caller.ShowError("Игра не найдена");
                return false;
            }

            if (session.Players.Count >= 2)
            {
                await Clients.Caller.ShowError("Игра уже полная");
                return false;
            }

            var player = new Player(Context.ConnectionId, playerName);

            if (_gameManager.AddPlayerToSession(sessionId, player))
            {
                await Groups.AddToGroupAsync(Context.ConnectionId, sessionId);

                // Уведомляем создателя игры
                await Clients.Group(sessionId).PlayerJoined(playerName, false);

                // Отправляем информацию о другом игроке новому участнику
                var otherPlayer = session.Players.FirstOrDefault(p => p.ConnectionId != Context.ConnectionId);
                if (otherPlayer != null)
                {
                    await Clients.Caller.PlayerJoined(otherPlayer.Name, false);
                }

                // Если оба игрока на месте, уведомляем о начале расстановки
                if (session.Players.Count == 2)
                {
                    await Clients.Group(sessionId).GameStarted();
                    await Clients.Group(sessionId).ShipsPlacementRequired();
                    await Clients.Group(sessionId).ShowMessage("Оба игрока в игре! Начинаем расстановку кораблей.");

                    // Обновляем список игроков
                    await UpdatePlayerList(session);
                }

                _logger.LogInformation("Игрок {PlayerName} присоединился к игре {SessionId}",
                    playerName, sessionId);

                return true;
            }

            await Clients.Caller.ShowError("Не удалось присоединиться к игре");
            return false;
        }

        // Размещение кораблей
        public async Task<bool> PlaceShips(string sessionId, List<Ship> ships)
        {
            if (!_gameManager.PlaceShips(sessionId, Context.ConnectionId, ships))
            {
                await Clients.Caller.ShowError("Невалидная расстановка кораблей");
                return false;
            }

            var session = _gameManager.GetSession(sessionId);
            if (session == null) return false;

            var player = session.GetPlayer(Context.ConnectionId);
            if (player == null) return false;

            // Уведомляем других игроков
            await Clients.OthersInGroup(sessionId).ShipsPlaced(player.Name);

            // Если оба игрока разместили корабли, начинаем игру
            if (session.AreBothPlayersReady() && session.State == GameState.InProgress)
            {
                await Clients.Group(sessionId).ShowMessage("Оба игрока готовы! Игра начинается.");
                await Clients.Group(sessionId).GameStateChanged(GameState.InProgress);

                // Уведомляем, чей первый ход
                var currentPlayer = session.CurrentTurnPlayer;
                if (currentPlayer != null)
                {
                    await Clients.Group(sessionId).ChangeTurn(currentPlayer.Name);
                    await Clients.Group(sessionId).ShowMessage($"Первым ходит: {currentPlayer.Name}");
                }
            }

            return true;
        }

        // Совершение выстрела
        public async Task Shoot(string sessionId, int x, int y)
        {
            var result = _gameManager.ProcessShot(sessionId, Context.ConnectionId, x, y);

            if (!result.IsValid)
            {
                await Clients.Caller.ShowError(result.Error ?? "Неверный выстрел");
                return;
            }

            var session = _gameManager.GetSession(sessionId);
            if (session == null) return;

            var shooter = session.GetPlayer(Context.ConnectionId);
            var target = session.GetOpponent(Context.ConnectionId);

            if (shooter == null || target == null) return;

            // Отправляем результат стрелявшему
            await Clients.Caller.ShotResult(x, y, result.IsHit, result.IsShipDestroyed, result.ShipName);

            // Уведомляем цель о выстреле (если это не конец игры)
            if (!result.IsGameOver)
            {
                await Clients.Client(target.ConnectionId).ReceiveShot(x, y);
            }

            // Если игра окончена
            if (result.IsGameOver)
            {
                await Clients.Group(sessionId).GameEnded(shooter.Name, shooter.ConnectionId);
                await Clients.Group(sessionId).ShowMessage($"Игра окончена! Победитель: {shooter.Name}");
                await Clients.Group(sessionId).GameStateChanged(GameState.Finished);

                // Удаляем игроков из группы
                await Groups.RemoveFromGroupAsync(shooter.ConnectionId, sessionId);
                await Groups.RemoveFromGroupAsync(target.ConnectionId, sessionId);
            }
            else
            {
                // Меняем ход, если нужно
                if (!result.ShooterKeepsTurn)
                {
                    session.SwitchTurn();
                    var nextPlayer = session.CurrentTurnPlayer;
                    if (nextPlayer != null)
                    {
                        await Clients.Group(sessionId).ChangeTurn(nextPlayer.Name);
                        await Clients.Group(sessionId).ShowMessage($"Следующий ход: {nextPlayer.Name}");
                    }
                }
                else
                {
                    // Ход остается у текущего игрока
                    await Clients.Group(sessionId).ShowMessage($"Попадание! {shooter.Name} ходит еще раз");
                }
            }
        }

        // Выход из игры
        public async Task LeaveGame(string sessionId)
        {
            var session = _gameManager.GetSession(sessionId);
            if (session == null) return;

            var player = session.GetPlayer(Context.ConnectionId);
            if (player == null) return;

            // Уведомляем других игроков
            await Clients.OthersInGroup(sessionId).PlayerLeft(player.Name);
            await Clients.OthersInGroup(sessionId).ShowMessage($"Игрок {player.Name} покинул игру");

            // Удаляем из группы
            await Groups.RemoveFromGroupAsync(Context.ConnectionId, sessionId);

            // Удаляем игрока из менеджера
            _gameManager.RemovePlayer(Context.ConnectionId);

            _logger.LogInformation("Игрок {PlayerName} покинул игру {SessionId}",
                player.Name, sessionId);
        }

        // Получение информации об игре
        public async Task<GameSession?> GetGameInfo(string sessionId)
        {
            return _gameManager.GetSession(sessionId);
        }

        // Получение списка активных игр
        public async Task<List<GameSession>> GetActiveGames()
        {
            return _gameManager.GetActiveSessions();
        }

        // Вспомогательный метод для обновления списка игроков
        private async Task UpdatePlayerList(GameSession session)
        {
            var playerInfos = session.Players.Select(p => new PlayerInfo
            {
                ConnectionId = p.ConnectionId,
                Name = p.Name,
                IsReady = p.IsReady,
                IsYourTurn = p.IsMyTurn
            }).ToList();

            await Clients.Group(session.SessionId).UpdatePlayerList(playerInfos);
        }

        // Обработка отключения клиента
        public override async Task OnDisconnectedAsync(Exception? exception)
        {
            _logger.LogInformation("Клиент отключился: {ConnectionId}", Context.ConnectionId);

            // Находим сессию игрока
            var session = _gameManager.GetSessionByPlayerConnection(Context.ConnectionId);
            if (session != null)
            {
                var player = session.GetPlayer(Context.ConnectionId);
                if (player != null)
                {
                    // Уведомляем других игроков
                    await Clients.OthersInGroup(session.SessionId).PlayerLeft(player.Name);
                    await Clients.OthersInGroup(session.SessionId).ShowMessage($"Игрок {player.Name} отключился");

                    // Если игра в процессе, отмечаем победителем другого игрока
                    if (session.State == GameState.InProgress || session.State == GameState.PlacingShips)
                    {
                        var opponent = session.GetOpponent(Context.ConnectionId);
                        if (opponent != null)
                        {
                            await Clients.Group(session.SessionId).GameEnded(opponent.Name, opponent.ConnectionId);
                            await Clients.Group(session.SessionId).ShowMessage($"Игра окончена. Победитель: {opponent.Name}");
                        }
                    }

                    // Удаляем из группы
                    await Groups.RemoveFromGroupAsync(Context.ConnectionId, session.SessionId);
                }

                // Удаляем игрока из менеджера
                _gameManager.RemovePlayer(Context.ConnectionId);
            }

            await base.OnDisconnectedAsync(exception);
        }
    }
}