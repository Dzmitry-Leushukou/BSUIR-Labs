namespace Server.Services;

using Shared;

/// <summary>
/// Server-side business logic for Battleship game
/// Handles ship placement validation, move processing, and game rules
/// </summary>
public class BattleshipGameLogic
{
    private const int BOARD_SIZE = GameBoard.BOARD_SIZE;
    public static readonly int[] SHIP_SIZES = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };

    public bool ValidateShipPlacement(Ship ship, GameBoard board)
    {
        if (ship.Positions.Count != ship.Size)
        {
            Console.WriteLine($"[ValidateShipPlacement] Position count mismatch: {ship.Positions.Count} != {ship.Size}");
            return false;
        }

        // Check for duplicate positions within the ship
        var uniquePositions = new HashSet<Position>();
        foreach (var pos in ship.Positions)
        {
            if (!uniquePositions.Add(pos))
            {
                Console.WriteLine($"[ValidateShipPlacement] Duplicate position found: ({pos.X}, {pos.Y})");
                return false;
            }
        }

        foreach (var pos in ship.Positions)
        {
            if (pos.X < 0 || pos.X >= BOARD_SIZE || pos.Y < 0 || pos.Y >= BOARD_SIZE)
            {
                Console.WriteLine($"[ValidateShipPlacement] Position out of bounds: ({pos.X}, {pos.Y})");
                return false;
            }

            // Проверяем, что клетка не занята другим кораблем
            if (board.GetCell(pos.X, pos.Y) == CellState.Ship)
            {
                Console.WriteLine($"[ValidateShipPlacement] Cell already occupied by another ship: ({pos.X}, {pos.Y})");
                return false;
            }
        }

        if (!IsValidLine(ship.Positions))
        {
            Console.WriteLine($"[ValidateShipPlacement] Not a valid line");
            return false;
        }

        if (!HasValidBuffer(ship.Positions, board))
        {
            Console.WriteLine($"[ValidateShipPlacement] Invalid buffer zone");
            return false;
        }

        return true;
    }

    public bool PlaceShips(List<Ship> ships, GameBoard board)
    {
        // Сначала валидируем весь флот целиком
        LastValidationError = ValidateShipFleetDetailed(ships);
        if (!string.IsNullOrEmpty(LastValidationError))
        {
            Console.WriteLine($"[PlaceShips] Fleet validation failed: {LastValidationError}");
            return false;
        }

        // Создаем временную копию доски для проверки
        var tempBoard = new GameBoard();

        // Проверяем каждый корабль на временной доске
        foreach (var ship in ships)
        {
            if (!ValidateShipPlacement(ship, tempBoard))
            {
                LastValidationError = $"Invalid ship placement at position ({ship.Positions[0].X}, {ship.Positions[0].Y})";
                Console.WriteLine($"[PlaceShips] Ship placement validation failed: {LastValidationError}");
                return false;
            }

            // Размещаем корабль на временной доске
            foreach (var pos in ship.Positions)
            {
                tempBoard.SetCell(pos.X, pos.Y, CellState.Ship);
            }
            tempBoard.Ships.Add(ship);
        }

        // Все проверки пройдены, теперь размещаем на реальной доске
        board.Ships.Clear();
        // Очищаем доску
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            for (int j = 0; j < BOARD_SIZE; j++)
            {
                board.SetCell(i, j, CellState.Empty);
            }
        }

        // Копируем корабли из временной доски
        foreach (var ship in tempBoard.Ships)
        {
            board.Ships.Add(ship);
            foreach (var pos in ship.Positions)
            {
                board.SetCell(pos.X, pos.Y, CellState.Ship);
            }
        }

        LastValidationError = null;
        Console.WriteLine($"[PlaceShips] Successfully placed {ships.Count} ships");
        return true;
    }

    /// <summary>
    /// Validates that the fleet contains the correct number and sizes of ships.
    /// Returns an error message if validation fails, or null if validation succeeds.
    /// </summary>
    private string? ValidateShipFleetDetailed(List<Ship> ships)
    {
        if (ships == null)
        {
            return "Ships list cannot be null";
        }

        // Check if the number of ships matches
        if (ships.Count < SHIP_SIZES.Length)
        {
            Console.WriteLine($"[ValidateShipFleetDetailed] Not enough ships: {ships.Count} < {SHIP_SIZES.Length}");
            return $"Not enough ships placed. Required: {SHIP_SIZES.Length}, Got: {ships.Count}";
        }

        if (ships.Count > SHIP_SIZES.Length)
        {
            Console.WriteLine($"[ValidateShipFleetDetailed] Too many ships: {ships.Count} > {SHIP_SIZES.Length}");
            return $"Too many ships placed. Maximum: {SHIP_SIZES.Length}, Got: {ships.Count}";
        }

        // Check for duplicate positions across all ships
        var allPositions = new HashSet<Position>();
        foreach (var ship in ships)
        {
            var shipPositions = new HashSet<Position>();
            foreach (var pos in ship.Positions)
            {
                if (!shipPositions.Add(pos))
                {
                    Console.WriteLine($"[ValidateShipFleetDetailed] Ship {ship.Id} has duplicate position: ({pos.X}, {pos.Y})");
                    return $"Ship has duplicate positions";
                }

                if (!allPositions.Add(pos))
                {
                    Console.WriteLine($"[ValidateShipFleetDetailed] Position ({pos.X}, {pos.Y}) is used by multiple ships");
                    return $"Ships overlap at position ({pos.X}, {pos.Y})";
                }
            }
        }

        // Count ships by size in provided fleet
        var providedCounts = new Dictionary<int, int>();
        foreach (var ship in ships)
        {
            if (providedCounts.ContainsKey(ship.Size))
                providedCounts[ship.Size]++;
            else
                providedCounts[ship.Size] = 1;
        }

        // Count required ships by size
        var requiredCounts = new Dictionary<int, int>();
        foreach (var size in SHIP_SIZES)
        {
            if (requiredCounts.ContainsKey(size))
                requiredCounts[size]++;
            else
                requiredCounts[size] = 1;
        }

        Console.WriteLine($"[ValidateShipFleetDetailed] Provided counts: {string.Join(", ", providedCounts.Select(kvp => $"{kvp.Value}x{kvp.Key}"))}");
        Console.WriteLine($"[ValidateShipFleetDetailed] Required counts: {string.Join(", ", requiredCounts.Select(kvp => $"{kvp.Value}x{kvp.Key}"))}");

        // Check if all required sizes have the correct count
        foreach (var requiredSize in requiredCounts.Keys)
        {
            if (!providedCounts.ContainsKey(requiredSize) ||
                providedCounts[requiredSize] != requiredCounts[requiredSize])
            {
                var shipSizeCounts = providedCounts.OrderByDescending(kvp => kvp.Key)
                    .Select(kvp => $"{kvp.Value}x{kvp.Key}");
                var errorMsg = "Invalid fleet composition. Required: 1x4, 2x3, 3x2, 4x1. " +
                    $"Got: {string.Join(", ", shipSizeCounts)}";
                Console.WriteLine($"[ValidateShipFleetDetailed] Fleet composition error: {errorMsg}");
                return errorMsg;
            }
        }

        // Check for unexpected ship sizes
        foreach (var providedSize in providedCounts.Keys)
        {
            if (!requiredCounts.ContainsKey(providedSize))
            {
                var shipSizeCounts = providedCounts.OrderByDescending(kvp => kvp.Key)
                    .Select(kvp => $"{kvp.Value}x{kvp.Key}");
                var errorMsg = "Invalid fleet composition. Required: 1x4, 2x3, 3x2, 4x1. " +
                    $"Got: {string.Join(", ", shipSizeCounts)}";
                Console.WriteLine($"[ValidateShipFleetDetailed] Unexpected ship size: {errorMsg}");
                return errorMsg;
            }
        }

        // Validate each ship's shape
        foreach (var ship in ships)
        {
            if (!IsValidLine(ship.Positions))
            {
                return $"Ship at ({ship.Positions[0].X}, {ship.Positions[0].Y}) has invalid shape - must be straight line";
            }
        }

        Console.WriteLine($"[ValidateShipFleetDetailed] Fleet validation passed");
        return null; // Validation passed
    }

    /// <summary>
    /// Validates that the fleet contains the correct number and sizes of ships (legacy method for backward compatibility)
    /// </summary>
    private bool ValidateShipFleet(List<Ship> ships)
    {
        return ValidateShipFleetDetailed(ships) == null;
    }

    public (bool isHit, bool isShipSunk, Ship? sunkShip, bool isGameOver) ProcessMove(Position targetPosition, GameBoard targetBoard)
    {
        if (targetPosition.X < 0 || targetPosition.X >= BOARD_SIZE ||
            targetPosition.Y < 0 || targetPosition.Y >= BOARD_SIZE)
            return (false, false, null, false);

        CellState currentState = targetBoard.GetCell(targetPosition.X, targetPosition.Y);

        if (currentState == CellState.Hit || currentState == CellState.Miss)
            return (false, false, null, false);

        bool isHit = currentState == CellState.Ship;

        if (isHit)
        {
            targetBoard.SetCell(targetPosition.X, targetPosition.Y, CellState.Hit);
            var ship = FindShipAtPosition(targetPosition, targetBoard);
            if (ship != null)
            {
                ship.HitsCount++;
                if (ship.IsSunk)
                {
                    bool isGameOver = targetBoard.Ships.All(s => s.IsSunk);
                    return (true, true, ship, isGameOver);
                }
            }
        }
        else
        {
            targetBoard.SetCell(targetPosition.X, targetPosition.Y, CellState.Miss);
        }

        return (isHit, false, null, false);
    }

    public bool HasPlayerWon(GameBoard opponentBoard)
    {
        return opponentBoard.Ships.All(s => s.IsSunk) && opponentBoard.Ships.Count > 0;
    }

    private bool IsValidLine(List<Position> positions)
    {
        if (positions.Count <= 1)
            return true;

        // Check for vertical line (all X are the same)
        bool isVertical = positions.All(p => p.X == positions[0].X);
        if (isVertical)
        {
            var sortedPositions = positions.OrderBy(p => p.Y).ToList();
            for (int i = 0; i < sortedPositions.Count - 1; i++)
            {
                if (sortedPositions[i + 1].Y - sortedPositions[i].Y != 1)
                    return false;
            }
            return true;
        }

        // Check for horizontal line (all Y are the same)
        bool isHorizontal = positions.All(p => p.Y == positions[0].Y);
        if (isHorizontal)
        {
            var sortedPositions = positions.OrderBy(p => p.X).ToList();
            for (int i = 0; i < sortedPositions.Count - 1; i++)
            {
                if (sortedPositions[i + 1].X - sortedPositions[i].X != 1)
                    return false;
            }
            return true;
        }

        return false;
    }

    private bool HasValidBuffer(List<Position> shipPositions, GameBoard board)
    {
        var shipPositionsSet = new HashSet<Position>(shipPositions);
        foreach (var shipPos in shipPositions)
        {
            for (int x = shipPos.X - 1; x <= shipPos.X + 1; x++)
            {
                for (int y = shipPos.Y - 1; y <= shipPos.Y + 1; y++)
                {
                    if (x >= 0 && x < GameBoard.BOARD_SIZE && y >= 0 && y < GameBoard.BOARD_SIZE)
                    {
                        var checkPos = new Position(x, y);
                        if (shipPositionsSet.Contains(checkPos))
                            continue;
                        if (board.GetCell(x, y) == CellState.Ship)
                        {
                            return false;
                        }
                    }
                }
            }
        }
        return true;
    }

    private Ship? FindShipAtPosition(Position position, GameBoard board)
    {
        return board.Ships.FirstOrDefault(s => s.Positions.Any(p => p.Equals(position)));
    }

    public string? LastValidationError { get; private set; }
}