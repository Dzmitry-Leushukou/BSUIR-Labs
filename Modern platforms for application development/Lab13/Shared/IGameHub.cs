namespace Shared;

/// <summary>
/// Strongly-typed Hub interface for Battleship game
/// Defines all methods that can be called from client and server
/// </summary>
public interface IGameHub
{
    // ===== Client Methods (called by server, implemented on client) =====
 
    /// <summary>
    /// Notifies client when a game has started
    /// </summary>
    Task GameStarted(GameStateDto gameState);

    /// <summary>
    /// Notifies client of the current game state
    /// </summary>
    Task GameStateChanged(GameStateDto gameState);

    /// <summary>
    /// Sends the board state to the client
    /// </summary>
    Task ReceiveBoard(BoardDto board);

    /// <summary>
    /// Notifies client of the result of their move
    /// </summary>
    Task MoveResult(MoveResultDto result);

    /// <summary>
    /// Notifies client of opponent's move
    /// </summary>
    Task OpponentMoveResult(Position targetPosition, bool isHit);

    /// <summary>
    /// Notifies client that game has ended
    /// </summary>
    Task GameEnded(string winnerId, Player winner);

    /// <summary>
    /// Notifies client of player list updates
    /// </summary>
    Task PlayerJoined(Player player);

    /// <summary>
    /// Notifies client of error messages
    /// </summary>
    Task ReceiveError(string message);

    /// <summary>
    /// Notifies client that opponent is ready
    /// </summary>
    Task OpponentReady();

 // ===== Server Methods (called by client, implemented on server) =====
    
    /// <summary>
    /// Registers a player and joins them to a game session
 /// </summary>
    Task JoinGame(string gameName, string playerName);

    /// <summary>
  /// Creates a new game session
    /// </summary>
    Task CreateGame(string gameName);

    /// <summary>
    /// Leaves the current game
    /// </summary>
    Task LeaveGame();

    /// <summary>
    /// Places ships on the board
    /// </summary>
    Task PlaceShips(List<Ship> ships);

    /// <summary>
    /// Marks player as ready to play
    /// </summary>
    Task ReadyToPlay();

    /// <summary>
    /// Makes a move (shoots at a position)
    /// </summary>
    Task MakeMove(Position targetPosition);

    /// <summary>
    /// Gets the current game state
    /// </summary>
    Task GetGameState();

    /// <summary>
    /// Gets the list of available games
    /// </summary>
    Task GetAvailableGames();
}
