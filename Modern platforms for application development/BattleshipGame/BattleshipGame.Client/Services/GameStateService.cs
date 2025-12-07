using BattleshipGame.Shared.Models;
using BattleshipGame.Shared.Validation;

namespace BattleshipGame.Client.Services
{
    public class GameStateService
    {
        private GameSession? _currentGame;
        private Player? _currentPlayer;
        private List<Ship> _ships = new();
        private readonly int _boardSize = 10;

        public event Action? OnGameStateChanged;
        public GameSession? CurrentGame => _currentGame;
        public Player? CurrentPlayer => _currentPlayer;
        public List<Ship> Ships => _ships;
        public int BoardSize => _boardSize;

        // Инициализация новой игры
        public void InitializeNewGame(string connectionId, string playerName)
        {
            _currentPlayer = new Player(connectionId, playerName);
            _ships = Player.GetDefaultShips();
            NotifyStateChanged();
        }

        // Установка текущей игры
        public void SetCurrentGame(GameSession game)
        {
            _currentGame = game;
            NotifyStateChanged();
        }

        public bool PlaceShip(int shipId, int startX, int startY, bool isHorizontal)
        {
            var ship = _ships.FirstOrDefault(s => s.Id == shipId);
            if (ship == null) return false;

            // Очищаем текущие клетки корабля
            ship.Cells.Clear();

            // Заполняем новые клетки
            for (int i = 0; i < ship.Size; i++)
            {
                int x = isHorizontal ? startX + i : startX;
                int y = isHorizontal ? startY : startY + i;

                // Проверка границ
                if (x < 0 || x >= _boardSize || y < 0 || y >= _boardSize)
                {
                    ship.Cells.Clear();
                    return false;
                }

                ship.Cells.Add(new Cell(x, y) { HasShip = true });
            }

            ship.IsHorizontal = isHorizontal;
            ship.IsPlaced = true;

            // Проверка на пересечение с другими кораблями
            var otherShips = _ships.Where(s => s.Id != shipId && s.IsPlaced);
            foreach (var otherShip in otherShips)
            {
                if (DoShipsOverlap(ship, otherShip))
                {
                    // Возвращаем старые клетки (но мы их уже очистили)
                    ship.Cells.Clear();
                    ship.IsPlaced = false;
                    return false;
                }
            }

            NotifyStateChanged();
            return true;
        }
        // Перемещение корабля
        public void MoveShip(int shipId, int deltaX, int deltaY)
        {
            var ship = _ships.FirstOrDefault(s => s.Id == shipId);
            if (ship == null || !ship.IsPlaced) return;

            var newCells = new List<Cell>();
            foreach (var cell in ship.Cells)
            {
                int newX = cell.X + deltaX;
                int newY = cell.Y + deltaY;

                // Проверка границ
                if (newX < 0 || newX >= _boardSize || newY < 0 || newY >= _boardSize)
                    return;

                newCells.Add(new Cell(newX, newY) { HasShip = true });
            }

            // Временно сохраняем старые клетки
            var oldCells = ship.Cells.ToList();
            ship.Cells = newCells;

            // Проверка на пересечение с другими кораблями
            var otherShips = _ships.Where(s => s.Id != shipId && s.IsPlaced);
            foreach (var otherShip in otherShips)
            {
                if (DoShipsOverlap(ship, otherShip))
                {
                    // Возвращаем старые клетки
                    ship.Cells = oldCells;
                    return;
                }
            }

            NotifyStateChanged();
        }

        // Поворот корабля
        public bool RotateShip(int shipId)
        {
            var ship = _ships.FirstOrDefault(s => s.Id == shipId);
            if (ship == null || !ship.IsPlaced || ship.Cells.Count == 0) return false;

            var firstCell = ship.Cells[0];
            return PlaceShip(shipId, firstCell.X, firstCell.Y, !ship.IsHorizontal);
        }

        // Случайная расстановка всех кораблей
        public void PlaceShipsRandomly()
        {
            var random = new Random();
            _ships = Player.GetDefaultShips();

            // Сбрасываем все корабли
            foreach (var ship in _ships)
            {
                ship.Cells.Clear();
                ship.IsPlaced = false;
            }

            // Расставляем корабли от большего к меньшему
            var shipsBySize = _ships.OrderByDescending(s => s.Size).ToList();

            foreach (var ship in shipsBySize)
            {
                bool placed = false;
                int attempts = 0;
                const int maxAttempts = 100;

                while (!placed && attempts < maxAttempts)
                {
                    attempts++;

                    // Случайные координаты и направление
                    int x = random.Next(0, _boardSize);
                    int y = random.Next(0, _boardSize);
                    bool horizontal = random.Next(0, 2) == 0;

                    // Пытаемся разместить
                    placed = PlaceShip(ship.Id, x, y, horizontal);
                }

                if (!placed)
                {
                    // Если не удалось разместить, начинаем заново
                    PlaceShipsRandomly();
                    return;
                }
            }

            NotifyStateChanged();
        }

        // Очистка расстановки
        public void ClearShips()
        {
            foreach (var ship in _ships)
            {
                ship.Cells.Clear();
                ship.IsPlaced = false;
            }
            NotifyStateChanged();
        }

        // Проверка, все ли корабли размещены
        public bool AreAllShipsPlaced()
        {
            return _ships.All(s => s.IsPlaced) &&
                   ShipValidator.ValidateAllShips(_ships);
        }

        // Получение клетки своего поля
        public Cell? GetOwnCell(int x, int y)
        {
            if (x < 0 || x >= _boardSize || y < 0 || y >= _boardSize)
                return null;

            foreach (var ship in _ships)
            {
                foreach (var cell in ship.Cells)
                {
                    if (cell.X == x && cell.Y == y)
                        return cell;
                }
            }

            return new Cell(x, y) { HasShip = false };
        }

        // Обработка выстрела по собственному полю
        public ShotResult ProcessOwnCellHit(int x, int y)
        {
            var cell = GetOwnCell(x, y);
            if (cell == null) return new ShotResult { IsValid = false };

            if (cell.HasShip && !cell.IsHit)
            {
                cell.IsHit = true;

                // Находим корабль
                var hitShip = _ships.FirstOrDefault(s => s.Cells.Any(c => c.X == x && c.Y == y));
                bool isDestroyed = hitShip?.IsDestroyed ?? false;

                return new ShotResult
                {
                    IsValid = true,
                    IsHit = true,
                    IsShipDestroyed = isDestroyed,
                    ShipName = hitShip?.Name
                };
            }

            return new ShotResult { IsValid = true, IsHit = false };
        }

        // Проверка пересечения кораблей
        private bool DoShipsOverlap(Ship ship1, Ship ship2)
        {
            foreach (var cell1 in ship1.Cells)
            {
                foreach (var cell2 in ship2.Cells)
                {
                    if (cell1.X == cell2.X && cell1.Y == cell2.Y)
                        return true;
                }
            }
            return false;
        }

        private void NotifyStateChanged()
        {
            OnGameStateChanged?.Invoke();
        }

        // Сброс состояния
        public void Reset()
        {
            _currentGame = null;
            _currentPlayer = null;
            _ships = new List<Ship>();
            NotifyStateChanged();
        }
    }

    public class ShotResult
    {
        public bool IsValid { get; set; }
        public bool IsHit { get; set; }
        public bool IsShipDestroyed { get; set; }
        public string? ShipName { get; set; }
    }
}