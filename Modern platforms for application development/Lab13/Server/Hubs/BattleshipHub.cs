namespace Server.Hubs;

using Shared;
using Server.Services;
using Microsoft.AspNetCore.SignalR;

/// <summary>
/// SignalR Hub for Battleship game - implements strongly-typed IGameHub interface
/// </summary>
public class BattleshipHub : Hub<IGameHub>
{
    private readonly GameSessionManager _gameSessionManager;
    private readonly ConnectionGameMapper _connectionGameMapper;

    public BattleshipHub(GameSessionManager gameSessionManager, ConnectionGameMapper connectionGameMapper)
    {
        _gameSessionManager = gameSessionManager;
        _connectionGameMapper = connectionGameMapper;
    }

    public async Task CreateGame(string gameName)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(gameName))
            {
                await Clients.Caller.ReceiveError("Game name cannot be empty");
                return;
            }

            var playerId = Context.ConnectionId;
            var playerName = Context.User?.Identity?.Name ?? "Player_" + playerId.Substring(0, Math.Min(6, playerId.Length));
            var gameSession = _gameSessionManager.CreateGame(gameName, playerId, playerName);

            _connectionGameMapper.MapConnection(playerId, gameSession.GameId);
            await Groups.AddToGroupAsync(Context.ConnectionId, gameSession.GameId);
            
            await Clients.Caller.GameStarted(new GameStateDto
            {
                GameId = gameSession.GameId,
                State = gameSession.State,
                CurrentPlayerId = gameSession.CurrentPlayerId,
                Player1 = gameSession.Player1,
                Player2 = gameSession.Player2
            });
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error creating game: {ex.Message}");
        }
    }

    public async Task JoinGame(string gameName, string playerName)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(playerName))
            {
                await Clients.Caller.ReceiveError("Player name cannot be empty");
                return;
            }

            var playerId = Context.ConnectionId;
            var availableGames = _gameSessionManager.GetAvailableGames();
            GameSession? gameSession = null;

            // Try to join an available game
            if (availableGames.Count > 0)
            {
                gameSession = _gameSessionManager.JoinGame(availableGames[0].GameId, playerId, playerName);
            }

            // If no available game, create a new one
            if (gameSession == null)
            {
                var gameNameToUse = string.IsNullOrWhiteSpace(gameName) 
                    ? "Game_" + DateTime.Now.Ticks 
                    : gameName;
                gameSession = _gameSessionManager.CreateGame(gameNameToUse, playerId, playerName);
            }

            _connectionGameMapper.MapConnection(playerId, gameSession.GameId);
            await Groups.AddToGroupAsync(Context.ConnectionId, gameSession.GameId);
            
            await Clients.Group(gameSession.GameId).GameStateChanged(new GameStateDto
            {
                GameId = gameSession.GameId,
                State = gameSession.State,
                CurrentPlayerId = gameSession.CurrentPlayerId,
                Player1 = gameSession.Player1,
                Player2 = gameSession.Player2
            });
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error joining game: {ex.Message}");
        }
    }

    public async Task GetAvailableGames()
    {
        try
        {
            var availableGames = _gameSessionManager.GetAvailableGames();
            // Note: IGameHub interface doesn't have a method to return game list
            // Using ReceiveError as informational message (could be improved with new method)
            await Clients.Caller.ReceiveError($"Available games: {availableGames.Count}");
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error getting available games: {ex.Message}");
        }
    }

    public async Task LeaveGame()
    {
        try
        {
            var gameId = _connectionGameMapper.GetGameId(Context.ConnectionId);
            if (!string.IsNullOrEmpty(gameId))
            {
                await Groups.RemoveFromGroupAsync(Context.ConnectionId, gameId);
                _gameSessionManager.RemovePlayerFromGame(gameId, Context.ConnectionId);
                _connectionGameMapper.RemoveConnection(Context.ConnectionId);
                
                // Notify other players in the game
                await Clients.Group(gameId).ReceiveError("A player has left the game");
            }
            await Clients.Caller.ReceiveError("Game left");
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error leaving game: {ex.Message}");
        }
    }

    public async Task PlaceShips(List<Ship> ships)
    {
        try
        {
            if (ships == null || ships.Count == 0)
            {
                await Clients.Caller.ReceiveError("Ships list cannot be empty");
                return;
            }

            var playerId = Context.ConnectionId;
            var gameId = _connectionGameMapper.GetGameId(playerId);

            if (string.IsNullOrEmpty(gameId))
            {
                await Clients.Caller.ReceiveError("Game ID not found. Please join a game first.");
                return;
            }

            var success = _gameSessionManager.PlaceShips(gameId, playerId, ships);

            if (success)
            {
                var gameSession = _gameSessionManager.GetGame(gameId);
                if (gameSession != null)
                {
                    await Clients.Group(gameId).GameStateChanged(new GameStateDto
                    {
                        GameId = gameSession.GameId,
                        State = gameSession.State,
                        CurrentPlayerId = gameSession.CurrentPlayerId,
                        Player1 = gameSession.Player1,
                        Player2 = gameSession.Player2
                    });
                }
            }
            else
            {
                string errorMessage = _gameSessionManager.GetLastValidationError() 
                    ?? "Invalid ship placement";
                await Clients.Caller.ReceiveError(errorMessage);
            }
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error placing ships: {ex.Message}");
        }
    }

    public async Task ReadyToPlay()
    {
        try
        {
            var playerId = Context.ConnectionId;
            var gameId = _connectionGameMapper.GetGameId(playerId);

            if (string.IsNullOrEmpty(gameId))
            {
                await Clients.Caller.ReceiveError("Game ID not found. Please join a game first.");
                return;
            }

            var success = _gameSessionManager.MarkPlayerReady(gameId, playerId);

            if (success)
            {
                var gameSession = _gameSessionManager.GetGame(gameId);
                if (gameSession != null)
                {
                    await Clients.Group(gameId).GameStateChanged(new GameStateDto
                    {
                        GameId = gameSession.GameId,
                        State = gameSession.State,
                        CurrentPlayerId = gameSession.CurrentPlayerId,
                        Player1 = gameSession.Player1,
                        Player2 = gameSession.Player2
                    });

                    if (gameSession.State == GameState.InProgress)
                    {
                        await Clients.Group(gameId).OpponentReady();
                    }
                }
            }
            else
            {
                await Clients.Caller.ReceiveError("Failed to mark as ready. Make sure you have placed all ships.");
            }
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error marking ready: {ex.Message}");
        }
    }

  public async Task MakeMove(Position targetPosition)
    {
        try
        {
            if (targetPosition == null)
            {
                await Clients.Caller.ReceiveError("Target position cannot be null");
                return;
            }

            var playerId = Context.ConnectionId;
            var gameId = _connectionGameMapper.GetGameId(playerId);

            if (string.IsNullOrEmpty(gameId))
            {
                await Clients.Caller.ReceiveError("Game ID not found. Please join a game first.");
                return;
            }

            var (success, moveResult) = _gameSessionManager.MakeMove(gameId, playerId, targetPosition);
   
    if (success && moveResult != null)
        {
 var gameSession = _gameSessionManager.GetGame(gameId);
        if (gameSession != null)
     {
       // ���������� ��������� �������� ������
         await Clients.Client(playerId).MoveResult(moveResult);
    
    // ���������� ���������� � ���� ����������
var opponentId = gameSession.Player1.Id == playerId ? gameSession.Player2?.Id : gameSession.Player1.Id;
 if (!string.IsNullOrEmpty(opponentId))
    {
     await Clients.Client(opponentId).OpponentMoveResult(targetPosition, moveResult.IsHit);
    }
    
     // ���������� ����� � ��������� ���������
          await Clients.Group(gameId).GameStateChanged(new GameStateDto
    {
     GameId = gameSession.GameId,
     State = gameSession.State,
      CurrentPlayerId = gameSession.CurrentPlayerId,
        Player1 = gameSession.Player1,
  Player2 = gameSession.Player2
 });
       
 if (moveResult.IsGameOver)
        {
    var winner = gameSession.Player1.Id == playerId ? gameSession.Player1 : gameSession.Player2!;
      await Clients.Group(gameId).GameEnded(playerId, winner);
    }
       }
 }
            else
            {
                await Clients.Caller.ReceiveError("Invalid move. It's not your turn or the position is already shot.");
            }
     }
    catch (Exception ex)
        {
  await Clients.Caller.ReceiveError($"Error making move: {ex.Message}");
        }
    }

    public async Task GetGameState()
    {
        try
        {
            var gameId = _connectionGameMapper.GetGameId(Context.ConnectionId);

            if (string.IsNullOrEmpty(gameId))
            {
                await Clients.Caller.ReceiveError("Game ID not found. Please join a game first.");
                return;
            }

            var gameSession = _gameSessionManager.GetGame(gameId);
            if (gameSession != null)
            {
                await Clients.Caller.GameStateChanged(new GameStateDto
                {
                    GameId = gameSession.GameId,
                    State = gameSession.State,
                    CurrentPlayerId = gameSession.CurrentPlayerId,
                    Player1 = gameSession.Player1,
                    Player2 = gameSession.Player2
                });
            }
            else
            {
                await Clients.Caller.ReceiveError("Game session not found");
            }
        }
        catch (Exception ex)
        {
            await Clients.Caller.ReceiveError($"Error getting game state: {ex.Message}");
        }
    }

    public override async Task OnDisconnectedAsync(Exception? exception)
    {
        try
        {
            var playerId = Context.ConnectionId;
            var gameId = _connectionGameMapper.GetGameId(playerId);

            if (!string.IsNullOrEmpty(gameId))
            {
                await Groups.RemoveFromGroupAsync(Context.ConnectionId, gameId);
                _gameSessionManager.RemovePlayerFromGame(gameId, playerId);
                await Clients.Group(gameId).ReceiveError("A player has disconnected");
            }

            _connectionGameMapper.RemoveConnection(playerId);
        }
        catch (Exception ex)
        {
            // Log error but don't throw - disconnection should always complete
            Console.WriteLine($"Error in OnDisconnectedAsync: {ex.Message}");
        }
        finally
        {
            await base.OnDisconnectedAsync(exception);
        }
    }
}
