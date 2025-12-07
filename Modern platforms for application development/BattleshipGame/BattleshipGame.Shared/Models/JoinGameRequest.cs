namespace BattleshipGame.Shared.Models
{
    public class JoinGameRequest
    {
        public string SessionId { get; set; } = string.Empty;
        public string PlayerName { get; set; } = string.Empty;
    }
}