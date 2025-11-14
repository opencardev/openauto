# PowerShell script to set up remote debugging from Windows
# Run this on your Windows machine

param(
    [string]$RpiHost = "rpi5.home.lan",
    [int]$DebugPort = 2345,
    [switch]$SetupTunnel,
    [switch]$TestConnection
)

Write-Host "=== OpenAuto Remote Debugging Setup (Windows) ===" -ForegroundColor Cyan
Write-Host ""

# Test connection to Pi
Write-Host "Testing connection to $RpiHost..." -ForegroundColor Yellow
$pingResult = Test-Connection -ComputerName $RpiHost -Count 1 -Quiet

if (-not $pingResult) {
    Write-Host "[X] Cannot reach $RpiHost" -ForegroundColor Red
    Write-Host "  Check that the Pi is on and network is configured" -ForegroundColor Red
    exit 1
}
Write-Host "[OK] Pi is reachable" -ForegroundColor Green

# Test SSH connection
Write-Host ""
Write-Host "Testing SSH connection..." -ForegroundColor Yellow
$null = ssh -o ConnectTimeout=5 -o BatchMode=yes "pi@${RpiHost}" "echo ok" 2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Host "[OK] SSH connection successful" -ForegroundColor Green
}
else {
    Write-Host "[X] SSH connection failed" -ForegroundColor Red
    Write-Host "  Make sure SSH keys are set up or use: ssh pi@$RpiHost" -ForegroundColor Yellow
    Write-Host "  You may need to accept the host key first" -ForegroundColor Yellow
}

# Copy setup script to Pi
Write-Host ""
Write-Host "Copying setup script to Pi..." -ForegroundColor Yellow
$scriptPath = Join-Path $PSScriptRoot "setup-remote-debug.sh"

if (Test-Path $scriptPath) {
    scp $scriptPath "pi@${RpiHost}:/tmp/setup-remote-debug.sh" 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[OK] Script copied successfully" -ForegroundColor Green
    }
    else {
        Write-Host "[X] Failed to copy script" -ForegroundColor Red
    }
}
else {
    Write-Host "[!] Setup script not found at: $scriptPath" -ForegroundColor Yellow
    Write-Host "  Skipping script copy" -ForegroundColor Yellow
}

if ($TestConnection) {
    Write-Host ""
    Write-Host "Testing debug port connection..." -ForegroundColor Yellow
    $tcpTest = Test-NetConnection -ComputerName $RpiHost -Port $DebugPort -WarningAction SilentlyContinue
    
    if ($tcpTest.TcpTestSucceeded) {
        Write-Host "[OK] Debug port $DebugPort is accessible" -ForegroundColor Green
    }
    else {
        Write-Host "[X] Cannot connect to debug port $DebugPort" -ForegroundColor Red
        Write-Host "  Make sure gdbserver is running on the Pi" -ForegroundColor Yellow
    }
}

if ($SetupTunnel) {
    Write-Host ""
    Write-Host "Setting up SSH tunnel for debugging..." -ForegroundColor Yellow
    Write-Host "  Local port: $DebugPort -> ${RpiHost}:$DebugPort" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Keep this terminal open while debugging!" -ForegroundColor Yellow
    Write-Host "Press Ctrl+C to close the tunnel" -ForegroundColor Yellow
    Write-Host ""
    
    ssh -L "${DebugPort}:localhost:${DebugPort}" "pi@${RpiHost}"
    exit
}

# Display next steps
Write-Host ""
Write-Host "=== Next Steps ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. On the Pi, run the setup script:" -ForegroundColor White
Write-Host "   ssh pi@$RpiHost" -ForegroundColor Gray
Write-Host "   chmod +x /tmp/setup-remote-debug.sh" -ForegroundColor Gray
Write-Host "   sudo /tmp/setup-remote-debug.sh" -ForegroundColor Gray
Write-Host ""
Write-Host "   The script will:" -ForegroundColor Gray
Write-Host "   - Install gdbserver and build tools" -ForegroundColor Gray
Write-Host "   - Clone OpenAuto repo (if needed)" -ForegroundColor Gray
Write-Host "   - Install all build dependencies" -ForegroundColor Gray
Write-Host "   - Build OpenAuto with debug symbols" -ForegroundColor Gray
Write-Host "   - Start gdbserver with your choice of mode" -ForegroundColor Gray
Write-Host ""
Write-Host "2. In VS Code:" -ForegroundColor White
Write-Host "   - Open the openauto workspace" -ForegroundColor Gray
Write-Host "   - Press F5 or go to Run > Start Debugging" -ForegroundColor Gray
Write-Host "   - Select 'Remote Debug on rpi5 (Launch)' or '(Attach)'" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Set breakpoints to find the hang:" -ForegroundColor White
Write-Host "   Files: src/autoapp/Service/Sensor/SensorService.cpp" -ForegroundColor Gray
Write-Host "   - Line in stop() where stopPolling is set" -ForegroundColor Gray
Write-Host "   - Line in sensorPolling() where it checks stopPolling" -ForegroundColor Gray
Write-Host "   - Line in onChannelError() at the start" -ForegroundColor Gray
Write-Host ""
Write-Host "   Also check: src/autoapp/Service/AndroidAutoEntity.cpp" -ForegroundColor Gray
Write-Host "   - stop() method" -ForegroundColor Gray
Write-Host "   - onChannelError() method" -ForegroundColor Gray
Write-Host ""
Write-Host "4. Reproduce the hang:" -ForegroundColor White
Write-Host "   - Let Android Auto connect and run normally" -ForegroundColor Gray
Write-Host "   - Click 'Exit' in Android Auto" -ForegroundColor Gray
Write-Host "   - Watch which breakpoints are hit" -ForegroundColor Gray
Write-Host "   - If it hangs, pause execution and check:" -ForegroundColor Gray
Write-Host "     * Thread states (View > Call Stack)" -ForegroundColor Gray
Write-Host "     * Value of stopPolling flag" -ForegroundColor Gray
Write-Host "     * Timer state" -ForegroundColor Gray
Write-Host "     * Which threads are still running" -ForegroundColor Gray
Write-Host ""

# Offer to open SSH connection
Write-Host "Would you like to connect to the Pi now? [Y/n]: " -NoNewline -ForegroundColor Yellow
$response = Read-Host

if ($response -eq "" -or $response -eq "Y" -or $response -eq "y") {
    Write-Host ""
    Write-Host "Connecting to $RpiHost..." -ForegroundColor Green
    ssh "pi@${RpiHost}"
}
