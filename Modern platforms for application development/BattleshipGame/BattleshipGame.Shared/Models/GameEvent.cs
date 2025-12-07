namespace BattleshipGame.Shared.Models
{
    public class GameEvent
    {
        public string Type { get; set; } = string.Empty; // "Shot", "ShipDestroyed", "GameOver", etc.
        public string Message { get; set; } = string.Empty;
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
        public object? Data { get; set; }

        public GameEvent() { }

        public GameEvent(string type, string message)
        {
            Type = type;
            Message = message;
        }

        public GameEvent(string type, string message, object data)
        {
            Type = type;
            Message = message;
            Data = data;
        }
    }
}