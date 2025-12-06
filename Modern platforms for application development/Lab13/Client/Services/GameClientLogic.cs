namespace Client.Services;

using Shared;

/// <summary>
/// Client-side game logic for Battleship game
/// Handles local game state, ship placement, and move processing
/// </summary>
public class GameClientLogic
{
    private GameBoard? _myBoard;
    private GameBoard? _opponentBoard;
    private GameState _gameState = GameState.WaitingForPlayers;
    private string? _currentPlayerId;
    private string? _playerId;

    public List<Ship> PlacedShips { get; private set; } = new();
    public List<Position> SelectedPositions { get; private set; } = new();
    public Ship? CurrentDraggingShip { get; private set; }
    public bool IsMyTurn { get; private set; }
    public int MyScore { get; set; }
    public int OpponentScore { get; set; }

    private static readonly int[] SHIP_SIZES = { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };

    /// <summary>
    /// Получить максимальный доступный размер корабля для размещения
    /// </summary>
    public int GetMaxAvailableShipSize()
    {
        var shipCounts = new Dictionary<int, int>();
        foreach (var size in SHIP_SIZES)
        {
            shipCounts[size] = shipCounts.GetValueOrDefault(size, 0) + 1;
        }

        foreach (var ship in PlacedShips)
        {
            shipCounts[ship.Size] = shipCounts.GetValueOrDefault(ship.Size, 0) - 1;
        }

        // Возвращаем максимальный размер, который еще доступен
        for (int size = 4; size >= 1; size--)
        {
            if (shipCounts.GetValueOrDefault(size, 0) > 0)
                return size;
        }

        return 0; // Нет доступных кораблей
    }

    /// <summary>
    /// Получить информацию о доступных кораблях
    /// </summary>
    public Dictionary<int, int> GetAvailableShipsInfo()
    {
        var shipCounts = new Dictionary<int, int>();
        foreach (var size in SHIP_SIZES)
        {
            shipCounts[size] = shipCounts.GetValueOrDefault(size, 0) + 1;
        }

        foreach (var ship in PlacedShips)
        {
            shipCounts[ship.Size] = shipCounts.GetValueOrDefault(ship.Size, 0) - 1;
        }

        return shipCounts;
    }

    /// <summary>
    /// Все ли корабли размещены
    /// </summary>
    public bool AreAllShipsPlaced()
    {
        return PlacedShips.Count == SHIP_SIZES.Length;
    }

    public void InitializeGame(string gameId, string playerId)
    {
        _playerId = playerId;
        _myBoard = new GameBoard();
        _opponentBoard = new GameBoard();
        PlacedShips.Clear();
        SelectedPositions.Clear();
        CurrentDraggingShip = null;
        IsMyTurn = false;
        MyScore = 0;
        OpponentScore = 0;
    }

    public void UpdateGameState(GameStateDto gameState)
    {
        _gameState = gameState.State;
        _currentPlayerId = gameState.CurrentPlayerId;
        IsMyTurn = _currentPlayerId == _playerId;
    }

    public void ToggleCellForPlacement(Position position)
    {
        // Проверяем, есть ли уже эта позиция в списке
        var existingPosition = SelectedPositions.FirstOrDefault(p => p.X == position.X && p.Y == position.Y);

        if (existingPosition != null)
        {
            Console.WriteLine($"[ToggleCellForPlacement] Removing position ({position.X},{position.Y})");
            SelectedPositions.Remove(existingPosition);
        }
        else
        {
            Console.WriteLine($"[ToggleCellForPlacement] Adding position ({position.X},{position.Y}). Total selected: {SelectedPositions.Count + 1}");
            SelectedPositions.Add(new Position(position.X, position.Y));
        }
    }

    public void ClearSelection()
    {
        SelectedPositions.Clear();
    }

    public bool AddShip(int shipSize)
    {
        Console.WriteLine($"[AddShip] Starting: size={shipSize}, selected positions={SelectedPositions.Count}");

        if (SelectedPositions.Count != shipSize)
        {
            Console.WriteLine($"[AddShip] FAILED: Position count mismatch: {SelectedPositions.Count} != {shipSize}");
            return false;
        }

        if (!IsValidLine(SelectedPositions))
        {
            Console.WriteLine($"[AddShip] FAILED: Selected positions don't form a valid line");
            return false;
        }

        // Проверяем, что корабль не пересекается с существующими кораблями или их буферами
        if (!IsValidShipPlacement(SelectedPositions))
        {
            Console.WriteLine($"[AddShip] FAILED: Ship placement violates buffer zones");
            return false;
        }

        Console.WriteLine($"[AddShip] SUCCESS: Creating ship with {SelectedPositions.Count} positions");

        var ship = new Ship
        {
            Id = PlacedShips.Count + 1,
            Size = shipSize,
            Positions = new List<Position>(SelectedPositions),
            HitsCount = 0
        };

        PlacedShips.Add(ship);
        Console.WriteLine($"[AddShip] Ship added. Total ships now: {PlacedShips.Count}");

        // Обновляем доску для отображения размещенного корабля
        if (_myBoard != null)
        {
            foreach (var pos in ship.Positions)
            {
                _myBoard.SetCell(pos.X, pos.Y, CellState.Ship);
            }
        }

        SelectedPositions.Clear();
        Console.WriteLine($"[AddShip] Selection cleared");

        return true;
    }

    public void RemoveShip(int shipId)
    {
        var ship = PlacedShips.FirstOrDefault(s => s.Id == shipId);
        if (ship != null)
        {
            // Удаляем с доски
            if (_myBoard != null)
            {
                foreach (var pos in ship.Positions)
                {
                    _myBoard.SetCell(pos.X, pos.Y, CellState.Empty);
                }
            }
            PlacedShips.RemoveAll(s => s.Id == shipId);
        }
    }

    public void ProcessMoveResult(MoveResultDto result)
    {
        if (result == null)
            return;

        if (_opponentBoard != null && result.TargetPosition != null)
        {
            if (result.TargetPosition.X >= 0 && result.TargetPosition.X < GameBoard.BOARD_SIZE &&
                result.TargetPosition.Y >= 0 && result.TargetPosition.Y < GameBoard.BOARD_SIZE)
            {
                _opponentBoard.SetCell(result.TargetPosition.X, result.TargetPosition.Y,
                    result.IsHit ? CellState.Hit : CellState.Miss);

                if (result.IsHit)
                    MyScore++;
            }
        }

        if (!result.IsHit)
            IsMyTurn = false;
    }

    public void ProcessOpponentMove(Position targetPosition, bool isHit)
    {
        if (targetPosition == null)
            return;

        if (_myBoard != null)
        {
            if (targetPosition.X >= 0 && targetPosition.X < GameBoard.BOARD_SIZE &&
                targetPosition.Y >= 0 && targetPosition.Y < GameBoard.BOARD_SIZE)
            {
                _myBoard.SetCell(targetPosition.X, targetPosition.Y,
                    isHit ? CellState.Hit : CellState.Miss);

                if (isHit)
                    OpponentScore++;
            }
        }

        IsMyTurn = true;
    }

    public CellState GetCellState(bool isMyBoard, int x, int y)
    {
        try
        {
            if (isMyBoard && _myBoard != null)
            {
                if (x >= 0 && x < GameBoard.BOARD_SIZE && y >= 0 && y < GameBoard.BOARD_SIZE)
                    return _myBoard.GetCell(x, y);
            }
            else if (!isMyBoard && _opponentBoard != null)
            {
                if (x >= 0 && x < GameBoard.BOARD_SIZE && y >= 0 && y < GameBoard.BOARD_SIZE)
                {
                    CellState state = _opponentBoard.GetCell(x, y);
                    return (state == CellState.Ship) ? CellState.Empty : state;
                }
            }
        }
        catch
        {
            // Игнорируем ошибки
        }

        return CellState.Empty;
    }

    public bool HasShip(int x, int y)
    {
        try
        {
            if (_myBoard != null && x >= 0 && x < GameBoard.BOARD_SIZE && y >= 0 && y < GameBoard.BOARD_SIZE)
                return _myBoard.GetCell(x, y) == CellState.Ship;
        }
        catch
        {
            // Игнорируем
        }
        return false;
    }

    public bool IsSelected(int x, int y)
    {
        return SelectedPositions.Any(p => p.X == x && p.Y == y);
    }

    public bool IsAlreadyShot(int x, int y)
    {
        try
        {
            if (_opponentBoard != null && x >= 0 && x < GameBoard.BOARD_SIZE && y >= 0 && y < GameBoard.BOARD_SIZE)
            {
                CellState state = _opponentBoard.GetCell(x, y);
                return state == CellState.Hit || state == CellState.Miss;
            }
        }
        catch
        {
            // Игнорируем
        }
        return false;
    }

    private bool IsValidLine(List<Position> positions)
    {
        if (positions.Count <= 1)
            return true;

        // Проверяем вертикальную линию (все X одинаковы)
        bool isVertical = positions.All(p => p.X == positions[0].X);
        if (isVertical)
        {
            var sorted = positions.OrderBy(p => p.Y).ToList();
            for (int i = 0; i < sorted.Count - 1; i++)
            {
                if (sorted[i + 1].Y - sorted[i].Y != 1)
                    return false;
            }
            return true;
        }

        // Проверяем горизонтальную линию (все Y одинаковы)
        bool isHorizontal = positions.All(p => p.Y == positions[0].Y);
        if (isHorizontal)
        {
            var sorted = positions.OrderBy(p => p.X).ToList();
            for (int i = 0; i < sorted.Count - 1; i++)
            {
                if (sorted[i + 1].X - sorted[i].X != 1)
                    return false;
            }
            return true;
        }

        return false;
    }

    private bool IsValidShipPlacement(List<Position> shipPositions)
    {
        Console.WriteLine($"[IsValidShipPlacement] Checking {shipPositions.Count} positions against {PlacedShips.Count} placed ships");
        var shipPositionsSet = new HashSet<Position>(shipPositions);
        foreach (var existingShip in PlacedShips)
        {
            foreach (var existingPos in existingShip.Positions)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        int checkX = existingPos.X + dx;
                        int checkY = existingPos.Y + dy;
                        if (shipPositionsSet.Contains(new Position(checkX, checkY)))
                        {
                            Console.WriteLine($"[IsValidShipPlacement] INVALID: New ship position ({checkX},{checkY}) conflicts with existing ship at ({existingPos.X},{existingPos.Y})");
                            return false;
                        }
                    }
                }
            }
        }
        Console.WriteLine($"[IsValidShipPlacement] VALID: No conflicts found");
        return true;
    }

    /// <summary>
    /// Автоматическая расстановка всех кораблей
    /// </summary>
    public bool AutoPlaceAllShips()
    {
        Console.WriteLine($"[AutoPlaceAllShips] Starting auto placement");

        if (_myBoard == null)
            return false;

        // Очищаем текущие корабли и доску
        PlacedShips.Clear();
        for (int i = 0; i < GameBoard.BOARD_SIZE; i++)
        {
            for (int j = 0; j < GameBoard.BOARD_SIZE; j++)
            {
                _myBoard.SetCell(i, j, CellState.Empty);
            }
        }

        Random rand = new Random();
        var shipsToPlace = new List<int> { 4, 3, 3, 2, 2, 2, 1, 1, 1, 1 };

        foreach (var size in shipsToPlace)
        {
            bool placed = false;
            int attempts = 0;
            int maxAttempts = 1000;

            while (!placed && attempts < maxAttempts)
            {
                attempts++;
                bool isHorizontal = rand.Next(2) == 0;

                // Генерируем начальную позицию с учетом ориентации и размера
                int maxX = isHorizontal ? GameBoard.BOARD_SIZE - size : GameBoard.BOARD_SIZE - 1;
                int maxY = isHorizontal ? GameBoard.BOARD_SIZE - 1 : GameBoard.BOARD_SIZE - size;

                if (maxX < 0 || maxY < 0)
                    continue;

                int x = rand.Next(0, maxX + 1);
                int y = rand.Next(0, maxY + 1);

                // Создаем позиции корабля
                var positions = new List<Position>();
                for (int i = 0; i < size; i++)
                {
                    if (isHorizontal)
                        positions.Add(new Position(x + i, y));
                    else
                        positions.Add(new Position(x, y + i));
                }

                // Проверяем, можно ли разместить корабль здесь
                if (IsValidShipPlacement(positions))
                {
                    var ship = new Ship
                    {
                        Id = PlacedShips.Count + 1,
                        Size = size,
                        Positions = positions,
                        HitsCount = 0
                    };

                    PlacedShips.Add(ship);
                    foreach (var pos in positions)
                    {
                        _myBoard.SetCell(pos.X, pos.Y, CellState.Ship);
                    }
                    placed = true;
                    Console.WriteLine($"[AutoPlaceAllShips] Successfully placed ship of size {size} at ({x},{y}), horizontal: {isHorizontal}");
                }
            }

            if (!placed)
            {
                Console.WriteLine($"[AutoPlaceAllShips] FAILED to place ship of size {size} after {maxAttempts} attempts");
                return false;
            }
        }

        Console.WriteLine($"[AutoPlaceAllShips] SUCCESS: All {PlacedShips.Count} ships placed");
        return true;
    }

    /// <summary>
    /// Начать построение корабля выбранного размера
    /// </summary>
    public bool StartBuildingShip(int shipSize, Position startPosition)
    {
        Console.WriteLine($"[StartBuildingShip] Starting to build ship of size {shipSize} at ({startPosition.X},{startPosition.Y})");

        // Очищаем текущий выбор
        SelectedPositions.Clear();

        // Добавляем начальную позицию
        SelectedPositions.Add(startPosition);

        return true;
    }

    /// <summary>
    /// Добавить позицию к текущему строящемуся кораблю
    /// </summary>
    public bool AddPositionToCurrentShip(Position position)
    {
        if (SelectedPositions.Count == 0)
            return false;

        // Проверяем, что позиция не была уже выбрана
        if (SelectedPositions.Any(p => p.X == position.X && p.Y == position.Y))
            return false;

        // Проверяем, что позиция смежная с последней выбранной
        var lastPosition = SelectedPositions.Last();
        bool isAdjacent = (Math.Abs(position.X - lastPosition.X) == 1 && position.Y == lastPosition.Y) ||
                         (Math.Abs(position.Y - lastPosition.Y) == 1 && position.X == lastPosition.X);

        if (!isAdjacent)
        {
            Console.WriteLine($"[AddPositionToCurrentShip] Position ({position.X},{position.Y}) is not adjacent to last position ({lastPosition.X},{lastPosition.Y})");
            return false;
        }

        SelectedPositions.Add(position);
        Console.WriteLine($"[AddPositionToCurrentShip] Added position ({position.X},{position.Y}). Total selected: {SelectedPositions.Count}");

        return true;
    }

    /// <summary>
    /// Завершить построение корабля
    /// </summary>
    public bool FinishBuildingShip(int shipSize)
    {
        if (SelectedPositions.Count != shipSize)
        {
            Console.WriteLine($"[FinishBuildingShip] Wrong number of positions: {SelectedPositions.Count} instead of {shipSize}");
            return false;
        }

        return AddShip(shipSize);
    }
}