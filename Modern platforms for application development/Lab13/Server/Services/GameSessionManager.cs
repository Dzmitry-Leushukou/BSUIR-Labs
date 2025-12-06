namespace Server.Services;

using Shared;

/// <summary>
/// Manages game sessions - handles creation, joining, and game state management
/// </summary>
public class GameSessionManager
{
    private readonly Dictionary<string, GameSession> _sessions = new();
    private readonly BattleshipGameLogic _gameLogic = new();
    private readonly object _lockObject = new();

    public GameSession CreateGame(string gameName, string playerId, string playerName)
    {
        var gameId = Guid.NewGuid().ToString();
        var gameSession = new GameSession
        {
            GameId = gameId,
            GameName = gameName,
            Player1 = new Player { Id = playerId, Name = playerName }
        };
        
        lock (_lockObject)
        {
            _sessions[gameId] = gameSession;
        }
        
        return gameSession;
    }

    public GameSession? JoinGame(string gameId, string playerId, string playerName)
    {
        lock (_lockObject)
        {
            if (!_sessions.TryGetValue(gameId, out var gameSession))
                return null;

            if (gameSession.IsFull)
                return null;

            gameSession.Player2 = new Player { Id = playerId, Name = playerName };
            gameSession.State = GameState.PlacingShips;
            gameSession.CurrentPlayerId = gameSession.Player1.Id;

            return gameSession;
        }
    }

    public GameSession? GetGame(string gameId)
    {
        lock (_lockObject)
        {
            _sessions.TryGetValue(gameId, out var gameSession);
            return gameSession;
        }
    }

    public List<GameSession> GetAvailableGames()
    {
        lock (_lockObject)
        {
            return _sessions.Values
                .Where(g => !g.IsFull && g.State == GameState.WaitingForPlayers)
                .ToList();
        }
    }

    public List<GameSession> GetActiveGames()
    {
        lock (_lockObject)
        {
            return _sessions.Values
                .Where(g => g.IsActive)
                .ToList();
        }
    }

    public bool PlaceShips(string gameId, string playerId, List<Ship> ships)
    {
        var gameSession = GetGame(gameId);
        if (gameSession == null)
        {
            Console.WriteLine($"[GameSessionManager.PlaceShips] Game session not found: {gameId}");
            return false;
        }

        if (gameSession.State != GameState.PlacingShips)
        {
            Console.WriteLine($"[GameSessionManager.PlaceShips] Invalid game state: {gameSession.State}");
            return false;
        }

        GameBoard? playerBoard = null;

        if (gameSession.Player1.Id == playerId)
            playerBoard = gameSession.Player1.Board;
        else if (gameSession.Player2?.Id == playerId)
            playerBoard = gameSession.Player2.Board;

        if (playerBoard == null)
        {
            Console.WriteLine($"[GameSessionManager.PlaceShips] Player not found in game: {playerId}");
            return false;
        }

        bool success = _gameLogic.PlaceShips(ships, playerBoard);
        if (success)
        {
            if (gameSession.Player1.Id == playerId)
                gameSession.Player1.IsReady = true;
            else if (gameSession.Player2?.Id == playerId)
                gameSession.Player2.IsReady = true;

            Console.WriteLine($"[GameSessionManager.PlaceShips] Player {playerId} successfully placed ships");
        }
        else
        {
            Console.WriteLine($"[GameSessionManager.PlaceShips] Failed to place ships for player {playerId}: {_gameLogic.LastValidationError}");
        }

        return success;
    }
 

    /// <summary>
    /// Gets the last validation error from the game logic
    /// </summary>
    public string? GetLastValidationError()
    {
        return _gameLogic.LastValidationError;
    }

    public bool MarkPlayerReady(string gameId, string playerId)
    {
        var gameSession = GetGame(gameId);
        if (gameSession == null)
            return false;

        if (gameSession.Player1.Id == playerId)
            gameSession.Player1.IsReady = true;
        else if (gameSession.Player2?.Id == playerId)
            gameSession.Player2.IsReady = true;
        else
            return false;

        // Start game when both players are ready
        if (gameSession.Player1.IsReady && gameSession.Player2?.IsReady == true)
        {
            gameSession.State = GameState.InProgress;
            gameSession.CurrentPlayerId = gameSession.Player1.Id;
        }

        return true;
    }

    public (bool success, MoveResultDto? result) MakeMove(string gameId, string playerId, Position targetPosition)
    {
        var gameSession = GetGame(gameId);
        if (gameSession == null || gameSession.State != GameState.InProgress)
            return (false, null);

        if (gameSession.CurrentPlayerId != playerId)
            return (false, null);

        GameBoard? opponentBoard = null;
        string? opponentId = null;

        if (gameSession.Player1.Id == playerId)
        {
            opponentBoard = gameSession.Player2?.Board;
            opponentId = gameSession.Player2?.Id;
        }
        else if (gameSession.Player2?.Id == playerId)
        {
            opponentBoard = gameSession.Player1.Board;
            opponentId = gameSession.Player1.Id;
        }

        if (opponentBoard == null || opponentId == null)
            return (false, null);

        var (isHit, isShipSunk, sunkShip, isGameOver) = _gameLogic.ProcessMove(targetPosition, opponentBoard);

        gameSession.MoveHistory.Add(new GameMove
        {
            PlayerId = playerId,
            TargetPosition = targetPosition,
            IsHit = isHit,
            Timestamp = DateTime.UtcNow
        });

        if (isHit)
        {
            if (gameSession.Player1.Id == playerId)
                gameSession.Player1.Score++;
            else
                gameSession.Player2!.Score++;
        }

        // Switch turn only if the shot missed
        if (!isHit)
        {
            gameSession.CurrentPlayerId = gameSession.CurrentPlayerId == gameSession.Player1.Id
                ? gameSession.Player2!.Id
                : gameSession.Player1.Id;
        }

        var moveResult = new MoveResultDto
        {
            TargetPosition = targetPosition,
            IsHit = isHit,
            IsShipSunk = isShipSunk,
            SunkShipId = sunkShip?.Id.ToString(),
            IsGameOver = isGameOver
        };

        if (isGameOver)
        {
            gameSession.State = GameState.Finished;
            moveResult.WinnerId = playerId;
        }

        return (true, moveResult);
    }

    public bool RemoveGame(string gameId)
    {
        lock (_lockObject)
        {
            return _sessions.Remove(gameId);
        }
    }

    public void RemovePlayerFromGame(string gameId, string playerId)
    {
        var gameSession = GetGame(gameId);
        if (gameSession == null)
            return;

        // Mark game as finished if it's still in progress
        if (gameSession.State != GameState.Finished)
        {
            gameSession.State = GameState.Finished;
        }

        // Remove game if only one player was in it, or if both players left
        if (gameSession.Player1.Id == playerId && gameSession.Player2 == null)
        {
            RemoveGame(gameId);
        }
        else if (gameSession.Player2?.Id == playerId)
        {
            // If player2 leaves, we can keep the game for player1 to potentially rejoin
            // Or remove it - depending on requirements. For now, we'll remove it.
            RemoveGame(gameId);
        }
    }
}
