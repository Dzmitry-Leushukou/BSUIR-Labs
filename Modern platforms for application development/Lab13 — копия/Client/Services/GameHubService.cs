namespace Client.Services;

using Shared;
using Microsoft.AspNetCore.SignalR.Client;

public class GameHubService : IAsyncDisposable
{
    private HubConnection? _hubConnection;
    private readonly HttpClient _httpClient;

    public event Func<GameStateDto, Task>? OnGameStateChanged;
    public event Func<GameStateDto, Task>? OnGameStarted;
    public event Func<BoardDto, Task>? OnBoardReceived;
    public event Func<MoveResultDto, Task>? OnMoveResult;
    public event Func<Position, bool, Task>? OnOpponentMoveResult;
    public event Func<string, Player, Task>? OnGameEnded;
    public event Func<string, Task>? OnErrorReceived;
    public event Func<Task>? OnOpponentReady;

    public GameHubService(HttpClient httpClient)
    {
        _httpClient = httpClient;
    }

    public async Task ConnectAsync(Uri baseUri)
    {
        if (_hubConnection != null && _hubConnection.State == HubConnectionState.Connected)
         return;

        // Определяем URL для SignalR Hub
        // Если клиент на 7240, подключаемся к серверу на 7055
        string hubUrl;
  
        if (baseUri.Host == "localhost" || baseUri.Host == "127.0.0.1")
    {
     // Для локальной разработки
            if (baseUri.Port == 7240 || baseUri.Port == 5245)
{
             // Это Blazor Client, подключаемся к Server на 7055
                hubUrl = "https://localhost:7055/battleshipHub";
            }
  else
            {
                // Сервер хостирует клиент, подключаемся локально
       hubUrl = baseUri.ToString().TrimEnd('/') + "/battleshipHub";
  }
        }
        else
        {
 // Production - подключаемся к тому же базовому адресу
      hubUrl = baseUri.ToString().TrimEnd('/') + "/battleshipHub";
        }

      _hubConnection = new HubConnectionBuilder()
        .WithUrl(hubUrl)
       .WithAutomaticReconnect()
         .Build();

      _hubConnection.On<GameStateDto>("GameStateChanged", async (gameState) =>
      {
    if (OnGameStateChanged != null)
      await OnGameStateChanged.Invoke(gameState);
    });

   _hubConnection.On<GameStateDto>("GameStarted", async (gameState) =>
        {
  if (OnGameStarted != null)
        await OnGameStarted.Invoke(gameState);
      });

        _hubConnection.On<BoardDto>("ReceiveBoard", async (board) =>
        {
            if (OnBoardReceived != null)
      await OnBoardReceived.Invoke(board);
     });

     _hubConnection.On<MoveResultDto>("MoveResult", async (result) =>
        {
   if (OnMoveResult != null)
  await OnMoveResult.Invoke(result);
    });

        _hubConnection.On<Position, bool>("OpponentMoveResult", async (targetPosition, isHit) =>
        {
     if (OnOpponentMoveResult != null)
      await OnOpponentMoveResult.Invoke(targetPosition, isHit);
        });

_hubConnection.On<string, Player>("GameEnded", async (winnerId, winner) =>
  {
    if (OnGameEnded != null)
       await OnGameEnded.Invoke(winnerId, winner);
        });

        _hubConnection.On<string>("ReceiveError", async (message) =>
        {
  if (OnErrorReceived != null)
    await OnErrorReceived.Invoke(message);
        });

        _hubConnection.On("OpponentReady", async () =>
        {
          if (OnOpponentReady != null)
      await OnOpponentReady.Invoke();
      });

    try
    {
      await _hubConnection.StartAsync();
    }
    catch (Exception ex)
    {
        Console.WriteLine($"[SignalR] Connection error: {ex}");
        throw new InvalidOperationException($"Failed to connect to hub at {hubUrl}: {ex.Message}", ex);
    }
    }

    public async Task DisconnectAsync()
    {
        if (_hubConnection != null)
   {
   await _hubConnection.StopAsync();
            await _hubConnection.DisposeAsync();
            _hubConnection = null;
        }
    }

    public async Task CreateGameAsync(string gameName)
    {
        if (_hubConnection == null)
            throw new InvalidOperationException("Not connected to hub");
        await _hubConnection.InvokeAsync("CreateGame", gameName);
    }

    public async Task JoinGameAsync(string gameName, string playerName)
 {
        if (_hubConnection == null)
   throw new InvalidOperationException("Not connected to hub");
        await _hubConnection.InvokeAsync("JoinGame", gameName, playerName);
    }

    public async Task LeaveGameAsync()
    {
 if (_hubConnection == null)
   throw new InvalidOperationException("Not connected to hub");
        await _hubConnection.InvokeAsync("LeaveGame");
    }

  public async Task PlaceShipsAsync(List<Ship> ships)
 {
        if (_hubConnection == null)
      throw new InvalidOperationException("Not connected to hub");
  await _hubConnection.InvokeAsync("PlaceShips", ships);
    }

    public async Task ReadyToPlayAsync()
  {
        if (_hubConnection == null)
            throw new InvalidOperationException("Not connected to hub");
    await _hubConnection.InvokeAsync("ReadyToPlay");
    }

    public async Task MakeMoveAsync(Position targetPosition)
    {
        if (_hubConnection == null)
    throw new InvalidOperationException("Not connected to hub");
        await _hubConnection.InvokeAsync("MakeMove", targetPosition);
    }

    public async Task GetGameStateAsync()
    {
        if (_hubConnection == null)
            throw new InvalidOperationException("Not connected to hub");
        await _hubConnection.InvokeAsync("GetGameState");
    }

    public async Task GetAvailableGamesAsync()
    {
   if (_hubConnection == null)
            throw new InvalidOperationException("Not connected to hub");
        await _hubConnection.InvokeAsync("GetAvailableGames");
    }

    public bool IsConnected => _hubConnection?.State == HubConnectionState.Connected;

    public string? CurrentPlayerId => _hubConnection?.ConnectionId;

    public async ValueTask DisposeAsync()
    {
        await DisconnectAsync();
     GC.SuppressFinalize(this);
  }
}
