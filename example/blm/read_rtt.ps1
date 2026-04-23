$port = 9090
$duration = 15
$start = Get-Date
$client = New-Object System.Net.Sockets.TcpClient("localhost", $port)
$stream = $client.GetStream()
$buffer = New-Object byte[] 1024
$encoding = New-Object System.Text.UTF8Encoding
Write-Host "Connected to RTT server on port $port"

while (((Get-Date) - $start).TotalSeconds -lt $duration) {
    if ($stream.DataAvailable) {
        $read = $stream.Read($buffer, 0, $buffer.Length)
        $text = $encoding.GetString($buffer, 0, $read)
        Write-Host $text -NoNewline
    }
    Start-Sleep -Milliseconds 100
}
$client.Close()
