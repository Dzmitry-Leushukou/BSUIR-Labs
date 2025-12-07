using System.Text.Json.Serialization;

namespace BattleshipGame.Shared.Models
{
    public class GameSession
    {
        public string SessionId { get; set; } = string.Empty;
        public Player? Player1 { get; set; }
        public Player? Player2 { get; set; }

        [JsonIgnore]
        public Player? CurrentTurnPlayer => Players.FirstOrDefault(p => p.IsMyTurn);

        [JsonIgnore]
        public List<Player> Players => new List<Player?> { Player1, Player2 }
            .Where(p => p != null)
            .Select(p => p!)
            .ToList();

        public GameState State { get; set; } = GameState.WaitingForPlayers;
        public DateTime CreatedAt { get; set; } = DateTime.UtcNow;
        public string? WinnerConnectionId { get; set; }

        public bool TryAddPlayer(Player player)
        {
            if (Player1 == null)
            {
                Player1 = player;
                return true;
            }
            else if (Player2 == null)
            {
                Player2 = player;
                return true;
            }
            return false;
        }

        public Player? GetPlayer(string connectionId)
        {
            if (Player1?.ConnectionId == connectionId) return Player1;
            if (Player2?.ConnectionId == connectionId) return Player2;
            return null;
        }

        public Player? GetOpponent(string connectionId)
        {
            if (Player1?.ConnectionId == connectionId) return Player2;
            if (Player2?.ConnectionId == connectionId) return Player1;
            return null;
        }

        public bool IsPlayerInSession(string connectionId)
        {
            return Player1?.ConnectionId == connectionId || Player2?.ConnectionId == connectionId;
        }

        public void SwitchTurn()
        {
            if (Player1 != null && Player2 != null)
            {
                Player1.IsMyTurn = !Player1.IsMyTurn;
                Player2.IsMyTurn = !Player2.IsMyTurn;
            }
        }

        public void SetTurn(string connectionId)
        {
            if (Player1 != null)
                Player1.IsMyTurn = Player1.ConnectionId == connectionId;
            if (Player2 != null)
                Player2.IsMyTurn = Player2.ConnectionId == connectionId;
        }

        public bool IsPlayerTurn(string connectionId)
        {
            return Players.FirstOrDefault(p => p.ConnectionId == connectionId)?.IsMyTurn ?? false;
        }

        public bool AreBothPlayersReady()
        {
            return Player1?.HasPlacedShips == true && Player2?.HasPlacedShips == true;
        }
    }
}