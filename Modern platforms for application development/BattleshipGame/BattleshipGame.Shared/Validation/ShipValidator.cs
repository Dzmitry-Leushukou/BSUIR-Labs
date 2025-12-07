using BattleshipGame.Shared.Models;

namespace BattleshipGame.Shared.Validation
{
    public static class ShipValidator
    {
        public static bool ValidateShipPlacement(Ship ship, List<Ship> existingShips, int boardSize)
        {
            // Проверка границ
            foreach (var cell in ship.Cells)
            {
                if (cell.X < 0 || cell.X >= boardSize || cell.Y < 0 || cell.Y >= boardSize)
                    return false;
            }

            // Проверка касания и пересечения с другими кораблями
            foreach (var existingShip in existingShips)
            {
                foreach (var existingCell in existingShip.Cells)
                {
                    foreach (var newCell in ship.Cells)
                    {
                        // Проверка наложение
                        if (existingCell.X == newCell.X && existingCell.Y == newCell.Y)
                            return false;

                        // Проверка касания (по правилам нельзя касаться)
                        if (Math.Abs(existingCell.X - newCell.X) <= 1 &&
                            Math.Abs(existingCell.Y - newCell.Y) <= 1)
                            return false;
                    }
                }
            }

            return true;
        }

        public static bool ValidateAllShips(List<Ship> ships, int boardSize = 10)
        {
            // Правила морского боя: 1x4, 2x3, 3x2, 4x1
            var requiredShips = new Dictionary<int, int>
            {
                {4, 1},  // 1 корабль на 4 клетки
                {3, 2},  // 2 корабля на 3 клетки
                {2, 3},  // 3 корабля на 2 клетки
                {1, 4}   // 4 корабля на 1 клетку
            };

            var placedShips = new Dictionary<int, int>();

            foreach (var ship in ships)
            {
                if (!placedShips.ContainsKey(ship.Size))
                    placedShips[ship.Size] = 0;
                placedShips[ship.Size]++;

                // Проверка что корабль размещен
                if (!ship.IsPlaced || ship.Cells.Count != ship.Size)
                    return false;

                // Проверка что все клетки уникальны
                var distinctCells = ship.Cells.Select(c => new { c.X, c.Y }).Distinct();
                if (distinctCells.Count() != ship.Cells.Count)
                    return false;
            }

            // Проверка количества кораблей каждого типа
            foreach (var required in requiredShips)
            {
                if (!placedShips.ContainsKey(required.Key) ||
                    placedShips[required.Key] != required.Value)
                    return false;
            }

            // Проверка касания между всеми кораблями
            for (int i = 0; i < ships.Count; i++)
            {
                for (int j = i + 1; j < ships.Count; j++)
                {
                    foreach (var cell1 in ships[i].Cells)
                    {
                        foreach (var cell2 in ships[j].Cells)
                        {
                            // Проверка касания
                            if (Math.Abs(cell1.X - cell2.X) <= 1 &&
                                Math.Abs(cell1.Y - cell2.Y) <= 1)
                                return false;
                        }
                    }
                }
            }

            return true;
        }
    }
}