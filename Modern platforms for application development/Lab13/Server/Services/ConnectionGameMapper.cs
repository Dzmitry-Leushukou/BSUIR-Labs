namespace Server.Services;

using System.Collections.Concurrent;

/// <summary>
/// Maps SignalR connection IDs to game session IDs
/// </summary>
public class ConnectionGameMapper
{
    private readonly ConcurrentDictionary<string, string> _connectionToGame = new();

    public void MapConnection(string connectionId, string gameId)
    {
        _connectionToGame[connectionId] = gameId;
    }

    public string? GetGameId(string connectionId)
    {
        _connectionToGame.TryGetValue(connectionId, out var gameId);
        return gameId;
    }

    public void RemoveConnection(string connectionId)
    {
        _connectionToGame.TryRemove(connectionId, out _);
    }
}
