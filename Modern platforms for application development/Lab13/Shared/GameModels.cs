namespace Shared;

/// <summary>
/// Represents the state of a single cell on the game board
/// </summary>
public enum CellState
{
    Empty = 0,
    Ship = 1,
    Hit = 2,
    Miss = 3,
    ShipPlacementValid = 4,    // Для отображения валидной позиции корабля
    ShipPlacementInvalid = 5   // Для отображения невалидной позиции корабля
}
/// <summary>
/// Represents the state of the game
/// </summary>
public enum GameState
{
    WaitingForPlayers,
    PlacingShips,
    InProgress,
    Finished
}

/// <summary>
/// Represents a position on the game board
/// </summary>
public class Position
{
    public int X { get; set; }
    public int Y { get; set; }

    public Position() { }

    public Position(int x, int y)
    {
        X = x;
        Y = y;
    }

    public override bool Equals(object? obj)
    {
        if (obj is Position pos)
         return X == pos.X && Y == pos.Y;
        return false;
    }

    public override int GetHashCode()
    {
      return HashCode.Combine(X, Y);
    }
}

/// <summary>
/// Represents a ship on the game board
/// </summary>
public class Ship
{
    public int Id { get; set; }
 public int Size { get; set; }
    public List<Position> Positions { get; set; } = new();
    public int HitsCount { get; set; }

    public bool IsSunk => HitsCount >= Size;
}

/// <summary>
/// Represents the game board state
/// </summary>
public class GameBoard
{
    public const int BOARD_SIZE = 10;
    
    // Используем одномерный массив вместо двумерного (для сериализации)
    public CellState[] Cells { get; set; } = new CellState[BOARD_SIZE * BOARD_SIZE];
    public List<Ship> Ships { get; set; } = new();

    public GameBoard()
    {
 // Initialize all cells as empty
 for (int i = 0; i < Cells.Length; i++)
        {
  Cells[i] = CellState.Empty;
     }
    }
    
    /// <summary>
    /// Helper method to convert 2D coordinates to 1D array index
    /// </summary>
    public int GetIndex(int x, int y) => x * BOARD_SIZE + y;
    
    /// <summary>
    /// Helper method to get cell state
    /// </summary>
    public CellState GetCell(int x, int y)
    {
        int index = GetIndex(x, y);
        if (index >= 0 && index < Cells.Length)
            return Cells[index];
   return CellState.Empty;
    }
    
    /// <summary>
    /// Helper method to set cell state
    /// </summary>
    public void SetCell(int x, int y, CellState state)
    {
        int index = GetIndex(x, y);
    if (index >= 0 && index < Cells.Length)
Cells[index] = state;
    }
}

/// <summary>
/// Represents a player in the game
/// </summary>
public class Player
{
    public string Id { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
  public GameBoard Board { get; set; } = new();
    public int Score { get; set; }
    public bool IsReady { get; set; }
}

/// <summary>
/// Represents a game session
/// </summary>
public class GameSession
{
    public string GameId { get; set; } = string.Empty;
    public string GameName { get; set; } = string.Empty;
  public Player Player1 { get; set; } = new();
    public Player? Player2 { get; set; }
    public GameState State { get; set; } = GameState.WaitingForPlayers;
    public string CurrentPlayerId { get; set; } = string.Empty;
    public DateTime CreatedAt { get; set; } = DateTime.UtcNow;
    public List<GameMove> MoveHistory { get; set; } = new();

    public bool IsFull => Player2 != null;
    public bool IsActive => State != GameState.Finished;
}

/// <summary>
/// Represents a move in the game
/// </summary>
public class GameMove
{
    public string PlayerId { get; set; } = string.Empty;
    public Position TargetPosition { get; set; } = new();
    public bool IsHit { get; set; }
    public DateTime Timestamp { get; set; } = DateTime.UtcNow;
}

/// <summary>
/// DTO for sending board state to client (without ship positions if it's opponent's board)
/// </summary>
public class BoardDto
{
    public CellState[,] Cells { get; set; } = new CellState[GameBoard.BOARD_SIZE, GameBoard.BOARD_SIZE];
    public bool IsMyBoard { get; set; }
}

/// <summary>
/// DTO for game state updates
/// </summary>
public class GameStateDto
{
    public string GameId { get; set; } = string.Empty;
    public GameState State { get; set; }
    public string CurrentPlayerId { get; set; } = string.Empty;
    public Player Player1 { get; set; } = new();
    public Player? Player2 { get; set; }
}

/// <summary>
/// DTO for move result
/// </summary>
public class MoveResultDto
{
    public Position TargetPosition { get; set; } = new();
    public bool IsHit { get; set; }
    public bool IsShipSunk { get; set; }
    public string? SunkShipId { get; set; }
    public bool IsGameOver { get; set; }
    public string? WinnerId { get; set; }
}
