using Microsoft.AspNetCore.SignalR.Client;
using Microsoft.Extensions.Logging;
using BattleshipGame.Shared.Models;
using BattleshipGame.Shared.Hubs;
using System.Text.Json;
using Microsoft.AspNetCore.Components;

namespace BattleshipGame.Client.Services
{
    public class GameHubService : IAsyncDisposable
    {
        private HubConnection? _hubConnection;
        private readonly ILogger<GameHubService> _logger;
        private readonly NavigationManager _navigationManager;
        private string? _currentSessionId;
        private string? _playerName;

        // События для уведомления UI
        public event Action? OnConnectionStateChanged;
        public event Action<string>? OnMessageReceived;
        public event Action<string>? OnErrorReceived;
        public event Action<GameSession>? OnGameStateUpdated;
        public event Action<List<PlayerInfo>>? OnPlayerListUpdated;

        public bool IsConnected => _hubConnection?.State == HubConnectionState.Connected;
        public string? CurrentSessionId => _currentSessionId;
        public string? PlayerName => _playerName;

        public GameHubService(ILogger<GameHubService> logger, NavigationManager navigationManager)
        {
            _logger = logger;
            _navigationManager = navigationManager;
        }

        public async Task InitializeAsync(string hubUrl)
        {
            try
            {
                // Для разработки используем фиксированный URL
                var serverUrl = "https://localhost:5001";

                _hubConnection = new HubConnectionBuilder()
                    .WithUrl($"{serverUrl}/gamehub")
                    .WithAutomaticReconnect()
                    .Build();

                SetupHubHandlers();

                await _hubConnection!.StartAsync();

                OnMessageReceived?.Invoke("Подключено к серверу");

                NotifyStateChanged();
            }
            catch (Exception ex)
            {
                OnErrorReceived?.Invoke($"Не удалось подключиться: {ex.Message}");
            }
        }
        // Настройка обработчиков сообщений от сервера
        private void SetupHubHandlers()
        {
            if (_hubConnection == null) return;

            // Основные уведомления
            _hubConnection.On<string>("GameCreated", (sessionId) =>
            {
                _currentSessionId = sessionId;
                _logger.LogInformation("Игра создана: {SessionId}", sessionId);
                OnMessageReceived?.Invoke($"Игра создана! ID: {sessionId}");
                NotifyStateChanged();
            });

            _hubConnection.On<string, bool>("PlayerJoined", (playerName, isYou) =>
            {
                var message = isYou ? "Вы присоединились к игре" : $"Игрок {playerName} присоединился";
                _logger.LogInformation(message);
                OnMessageReceived?.Invoke(message);
                NotifyStateChanged();
            });

            _hubConnection.On<string>("PlayerLeft", (playerName) =>
            {
                _logger.LogInformation("Игрок покинул игру: {PlayerName}", playerName);
                OnMessageReceived?.Invoke($"Игрок {playerName} покинул игру");
                NotifyStateChanged();
            });

            _hubConnection.On("GameStarted", async () =>
            {
                _logger.LogInformation("Игра началась!");
                OnMessageReceived?.Invoke("Игра началась! Расставьте корабли.");
                NotifyStateChanged();
            });

            _hubConnection.On<GameState>("GameStateChanged", (state) =>
            {
                _logger.LogInformation("Состояние игры изменилось: {State}", state);
                OnMessageReceived?.Invoke($"Состояние: {GetStateDescription(state)}");
                NotifyStateChanged();
            });

            _hubConnection.On("ShipsPlacementRequired", () =>
            {
                _logger.LogInformation("Требуется расстановка кораблей");
                OnMessageReceived?.Invoke("Пожалуйста, расставьте корабли");
                NotifyStateChanged();
            });

            _hubConnection.On<string>("ShipsPlaced", (playerName) =>
            {
                _logger.LogInformation("Игрок разместил корабли: {PlayerName}", playerName);
                OnMessageReceived?.Invoke($"{playerName} разместил корабли");
                NotifyStateChanged();
            });

            // Выстрелы
            _hubConnection.On<int, int>("ReceiveShot", (x, y) =>
            {
                _logger.LogInformation("Получен выстрел по координатам: ({X},{Y})", x, y);
                OnMessageReceived?.Invoke($"Противник выстрелил в ({x},{y})");
                NotifyStateChanged();
            });

            _hubConnection.On<int, int, bool, bool, string>("ShotResult",
                (x, y, isHit, isShipDestroyed, shipName) =>
                {
                    var result = isHit ? "Попадание!" : "Промах";
                    var destroyed = isShipDestroyed ? $" Корабль {shipName} уничтожен!" : "";
                    _logger.LogInformation("Результат выстрела: {Result}{Destroyed}", result, destroyed);
                    OnMessageReceived?.Invoke($"{result} в ({x},{y}){destroyed}");
                    NotifyStateChanged();
                });

            _hubConnection.On<string>("ChangeTurn", (playerName) =>
            {
                _logger.LogInformation("Ход перешел к: {PlayerName}", playerName);
                OnMessageReceived?.Invoke($"Сейчас ходит: {playerName}");
                NotifyStateChanged();
            });

            // Конец игры
            _hubConnection.On<string, string>("GameEnded", (winnerName, winnerConnectionId) =>
            {
                _logger.LogInformation("Игра окончена! Победитель: {WinnerName}", winnerName);
                OnMessageReceived?.Invoke($"Игра окончена! Победитель: {winnerName}");
                NotifyStateChanged();
            });

            // Системные сообщения
            _hubConnection.On<string>("ShowMessage", (message) =>
            {
                _logger.LogInformation("Сообщение от сервера: {Message}", message);
                OnMessageReceived?.Invoke(message);
            });

            _hubConnection.On<List<PlayerInfo>>("UpdatePlayerList", (players) =>
            {
                _logger.LogInformation("Обновлен список игроков: {Count} игроков", players.Count);
                OnPlayerListUpdated?.Invoke(players);
                NotifyStateChanged();
            });

            _hubConnection.On<string>("ShowError", (error) =>
            {
                _logger.LogError("Ошибка от сервера: {Error}", error);
                OnErrorReceived?.Invoke(error);
            });

            // Обработка состояния подключения
            _hubConnection!.Closed += async (error) =>
            {
                _logger.LogWarning(error, "Соединение закрыто");
                OnMessageReceived?.Invoke("Соединение потеряно. Попытка переподключения...");
                NotifyStateChanged();
                await Task.Delay(new Random().Next(0, 5) * 1000);
                try
                {
                    await _hubConnection.StartAsync();
                    _logger.LogInformation("Переподключение успешно");
                    OnMessageReceived?.Invoke("Соединение восстановлено!");
                    NotifyStateChanged();
                }
                catch (Exception ex)
                {
                    _logger.LogError(ex, "Не удалось переподключиться");
                }
            };

            _hubConnection.Reconnecting += (error) =>
            {
                _logger.LogWarning(error, "Переподключение...");
                OnMessageReceived?.Invoke("Переподключение...");
                NotifyStateChanged();
                return Task.CompletedTask;
            };

            _hubConnection.Reconnected += (connectionId) =>
            {
                _logger.LogInformation("Переподключено с ConnectionId: {ConnectionId}", connectionId);
                OnMessageReceived?.Invoke("Соединение восстановлено!");
                NotifyStateChanged();
                return Task.CompletedTask;
            };
        }

        // Методы для вызова на сервере
        public async Task<string> CreateGameAsync(string playerName)
        {
            if (_hubConnection == null)
                throw new InvalidOperationException("Подключение не установлено");

            _playerName = playerName;
            var sessionId = await _hubConnection.InvokeAsync<string>("CreateGame", playerName);
            _currentSessionId = sessionId;
            return sessionId;
        }

        public async Task<bool> JoinGameAsync(string sessionId, string playerName)
        {
            if (_hubConnection == null)
                throw new InvalidOperationException("Подключение не установлено");

            _playerName = playerName;
            _currentSessionId = sessionId;
            return await _hubConnection.InvokeAsync<bool>("JoinGame", sessionId, playerName);
        }

        public async Task<bool> PlaceShipsAsync(List<Ship> ships)
        {
            if (_hubConnection == null || string.IsNullOrEmpty(_currentSessionId))
                throw new InvalidOperationException("Не подключен к игре");

            return await _hubConnection.InvokeAsync<bool>("PlaceShips", _currentSessionId, ships);
        }

        public async Task ShootAsync(int x, int y)
        {
            if (_hubConnection == null || string.IsNullOrEmpty(_currentSessionId))
                throw new InvalidOperationException("Не подключен к игре");

            await _hubConnection.InvokeAsync("Shoot", _currentSessionId, x, y);
        }

        public async Task LeaveGameAsync()
        {
            if (_hubConnection == null || string.IsNullOrEmpty(_currentSessionId)) return;

            await _hubConnection.InvokeAsync("LeaveGame", _currentSessionId);
            _currentSessionId = null;
        }

        public async Task<GameSession?> GetGameInfoAsync(string sessionId)
        {
            if (_hubConnection == null || !IsConnected) return null;

            try
            {
                return await _hubConnection.InvokeAsync<GameSession?>("GetGameInfo", sessionId);
            }
            catch (Exception)
            {
                // Если игра не найдена или произошла ошибка, возвращаем null
                return null;
            }
        }

        public async Task<List<GameSession>> GetActiveGamesAsync()
        {
            if (_hubConnection == null) return new List<GameSession>();

            return await _hubConnection.InvokeAsync<List<GameSession>>("GetActiveGames");
        }

        // Вспомогательные методы
        private void NotifyStateChanged()
        {
            OnConnectionStateChanged?.Invoke();
        }

        private string GetStateDescription(GameState state)
        {
            return state switch
            {
                GameState.WaitingForPlayers => "Ожидание игроков",
                GameState.PlacingShips => "Расстановка кораблей",
                GameState.InProgress => "Игра в процессе",
                GameState.Finished => "Игра завершена",
                _ => "Неизвестно"
            };
        }

        public async ValueTask DisposeAsync()
        {
            if (_hubConnection != null)
            {
                await _hubConnection.DisposeAsync();
            }
        }
    }
}