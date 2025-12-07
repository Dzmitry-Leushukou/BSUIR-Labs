using BattleshipGame.Shared.Models;

namespace BattleshipGame.Shared.Hubs
{
    public interface IGameClient
    {
        // Уведомления о состоянии игры
        Task GameCreated(string sessionId);
        Task PlayerJoined(string playerName, bool isYou);
        Task PlayerLeft(string playerName);
        Task GameStarted();
        Task GameStateChanged(GameState state);

        // Расстановка кораблей
        Task ShipsPlacementRequired();
        Task ShipsPlaced(string playerName);

        // Выстрелы
        Task ReceiveShot(int x, int y);
        Task ShotResult(int x, int y, bool isHit, bool isShipDestroyed, string? shipName);
        Task ChangeTurn(string playerName);

        // Конец игры
        Task GameEnded(string winnerName, string winnerConnectionId);

        // Системные сообщения
        Task ShowMessage(string message);
        Task UpdatePlayerList(List<PlayerInfo> players);

        // Ошибки
        Task ShowError(string error);
    }

    // Дополнительный класс для информации об игроке
    public class PlayerInfo
    {
        public string ConnectionId { get; set; } = string.Empty;
        public string Name { get; set; } = string.Empty;
        public bool IsReady { get; set; }
        public bool IsYourTurn { get; set; }
    }
}