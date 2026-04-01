$Host.UI.RawUI.WindowTitle = 'RTT Log Viewer'
$Host.UI.RawUI.BackgroundColor = 'Black'
$Host.UI.RawUI.ForegroundColor = 'Green'
Clear-Host

Write-Host "=== RTT Log Viewer Connected ===" -ForegroundColor Green

$client = New-Object System.Net.Sockets.TcpClient
$isConnected = $false

# 尝试连接，带简单重试
for ($i = 0; $i -lt 5; $i++) {
    try {
        $client.Connect('127.0.0.1', 9090)
        $isConnected = $true
        break
    } catch {
        Start-Sleep -Seconds 1
    }
}

if (-not $isConnected) {
    Write-Host "Failed to connect to RTT server on port 9090." -ForegroundColor Red
    Start-Sleep -Seconds 5
    exit
}

$stream = $client.GetStream()
$buffer = New-Object byte[] 4096

try {
    while ($true) {
        if ($stream.DataAvailable) {
            $bytesRead = $stream.Read($buffer, 0, 4096)
            if ($bytesRead -gt 0) {
                $text = [System.Text.Encoding]::UTF8.GetString($buffer, 0, $bytesRead)
                Write-Host $text -NoNewline
            }
        } else {
            Start-Sleep -Milliseconds 50
        }
    }
} catch {
    Write-Host "`nConnection closed or error occurred: $_" -ForegroundColor Yellow
} finally {
    if ($client) {
        $client.Close()
    }
    Start-Sleep -Seconds 3
}
