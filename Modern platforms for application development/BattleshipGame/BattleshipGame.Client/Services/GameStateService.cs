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
            ship.Cells.Clear();

            // Проверяем, помещается ли корабль в границы
            if (isHorizontal)
            {
                if (startX + ship.Size > _boardSize) return false;
            }
            else
            {
                if (startY + ship.Size > _boardSize) return false;
            }

            // Создаем временные клетки для проверки
            var tempCells = new List<Cell>();
            for (int i = 0; i < ship.Size; i++)
            {
                int x = isHorizontal ? startX + i : startX;
                int y = isHorizontal ? startY : startY + i;

                // Проверка границ
                if (x < 0 || x >= _boardSize || y < 0 || y >= _boardSize)
                    return false;

                tempCells.Add(new Cell(x, y) { HasShip = true });
            }

            // Проверяем наложение и касание с другими кораблями
            var otherShips = _ships.Where(s => s.Id != shipId && s.IsPlaced);
            foreach (var otherShip in otherShips)
            {
                if (DoShipsTouchOrOverlap(tempCells, otherShip.Cells))
                {
                    return false;
                }
            }

            // Если все проверки прошли, размещаем корабль
            ship.Cells = tempCells;
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
                const int maxAttempts = 500; // Увеличим лимит попыток

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

        // Проверка на касание или наложение кораблей
        private bool DoShipsTouchOrOverlap(List<Cell> cells1, List<Cell> cells2)
        {
            foreach (var cell1 in cells1)
            {
                foreach (var cell2 in cells2)
                {
                    // Проверка наложения (точное совпадение координат)
                    if (cell1.X == cell2.X && cell1.Y == cell2.Y)
                        return true;

                    // Проверка касания (расстояние 1 клетка по горизонтали/вертикали)
                    if (Math.Abs(cell1.X - cell2.X) <= 1 && Math.Abs(cell1.Y - cell2.Y) <= 1)
                        return true;
                }
            }
            return false;
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

        // Вспомогательный метод для получения корабля по координатам
        public Ship? GetShipAt(int x, int y)
        {
            return _ships.FirstOrDefault(s => s.Cells.Any(c => c.X == x && c.Y == y));
        }

        // Вспомогательный метод для получения направления корабля
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