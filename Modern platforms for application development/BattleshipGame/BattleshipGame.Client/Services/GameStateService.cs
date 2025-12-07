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

        // Расстановка корабля на поле с валидацией
        public bool PlaceShip(int shipId, int startX, int startY, bool isHorizontal)
        {
            var ship = _ships.FirstOrDefault(s => s.Id == shipId);
            if (ship == null) return false;

            // Очищаем текущие клетки корабля
            var oldCells = ship.Cells.ToList();
            ship.Cells.Clear();

            // Проверяем, помещается ли корабль в границы
            if (isHorizontal)
            {
                if (startX + ship.Size > _boardSize)
                {
                    ship.Cells = oldCells; // Восстанавливаем старые клетки
                    return false;
                }
            }
            else
            {
                if (startY + ship.Size > _boardSize)
                {
                    ship.Cells = oldCells;
                    return false;
                }
            }

            // Создаем новые клетки
            var newCells = new List<Cell>();
            for (int i = 0; i < ship.Size; i++)
            {
                int x = isHorizontal ? startX + i : startX;
                int y = isHorizontal ? startY : startY + i;

                newCells.Add(new Cell(x, y) { HasShip = true });
            }

            // Проверяем наложение и касание с другими кораблями
            var otherShips = _ships.Where(s => s.Id != shipId && s.IsPlaced);
            foreach (var otherShip in otherShips)
            {
                foreach (var existingCell in otherShip.Cells)
                {
                    foreach (var newCell in newCells)
                    {
                        // Проверка наложение (точное совпадение)
                        if (existingCell.X == newCell.X && existingCell.Y == newCell.Y)
                        {
                            ship.Cells = oldCells;
                            return false;
                        }

                        // Проверка касания (расстояние 1 клетка по диагонали тоже)
                        if (Math.Abs(existingCell.X - newCell.X) <= 1 &&
                            Math.Abs(existingCell.Y - newCell.Y) <= 1)
                        {
                            ship.Cells = oldCells;
                            return false;
                        }
                    }
                }
            }

            // Если все проверки прошли, размещаем корабль
            ship.Cells = newCells;
            ship.IsHorizontal = isHorizontal;
            ship.IsPlaced = true;

            NotifyStateChanged();
            return true;
        }

        // Поворот корабля
        public bool RotateShip(int shipId)
        {
            var ship = _ships.FirstOrDefault(s => s.Id == shipId);
            if (ship == null || !ship.IsPlaced || ship.Cells.Count == 0) return false;

            var firstCell = ship.Cells[0];
            return PlaceShip(shipId, firstCell.X, firstCell.Y, !ship.IsHorizontal);
        }

        // Случайная расстановка всех кораблей с валидацией
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
                const int maxAttempts = 1000;

                while (!placed && attempts < maxAttempts)
                {
                    attempts++;

                    // Случайные координаты и направление
                    int maxStartX = _boardSize - (ship.IsHorizontal ? ship.Size : 1);
                    int maxStartY = _boardSize - (ship.IsHorizontal ? 1 : ship.Size);

                    int x = random.Next(0, maxStartX + 1);
                    int y = random.Next(0, maxStartY + 1);
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
            return _ships.All(s => s.IsPlaced);
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

        // Получение корабля по координатам
        public Ship? GetShipAt(int x, int y)
        {
            return _ships.FirstOrDefault(s => s.Cells.Any(c => c.X == x && c.Y == y));
        }

        // Получение направления корабля
        public bool IsShipHorizontal(int shipId)
        {
            var ship = _ships.FirstOrDefault(s => s.Id == shipId);
            return ship?.IsHorizontal ?? true;
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
}