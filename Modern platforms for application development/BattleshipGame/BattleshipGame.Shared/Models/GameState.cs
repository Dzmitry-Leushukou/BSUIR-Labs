namespace BattleshipGame.Shared.Models
{
    public enum GameState
    {
        WaitingForPlayers,  // Ожидание игроков
        PlacingShips,       // Расстановка кораблей
        InProgress,         // Игра идет
        Finished            // Игра завершена
    }
}