# ==============================================================================
# Script ký số Authenticode cho MecchaChameleon (Giảm thiểu False Positive)
# ==============================================================================
param(
    [string]$ExePath = "D:\Tool-meccha\MecchaChameleon-External-Cheat-main\MecchaChameleon\MecchaChameleon\x64\Release\MecchaChameleon.exe"
)

Write-Host "[*] Kiem tra chung chi Code Signing..." -ForegroundColor Cyan

$certName = "Meccha Studio Development"
$cert = Get-ChildItem -Path Cert:\CurrentUser\My -CodeSigningCert | Where-Object { $_.Subject -match $certName } | Select-Object -First 1

if (-not $cert) {
    Write-Host "[+] Tao moi Self-Signed Code Signing Certificate..." -ForegroundColor Yellow
    $cert = New-SelfSignedCertificate `
        -Type CodeSigningCert `
        -Subject "CN=$certName, O=Meccha Studio" `
        -HashAlgorithm SHA256 `
        -KeyLength 2048 `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -NotAfter (Get-Date).AddYears(5)
}

Write-Host "[+] Su dung chung chi: $($cert.Thumbprint) ($($cert.Subject))" -ForegroundColor Green

if (Test-Path $ExePath) {
    Write-Host "[*] Dang ky so cho: $ExePath" -ForegroundColor Cyan
    $status = Set-AuthenticodeSignature -FilePath $ExePath -Certificate $cert -HashAlgorithm SHA256
    Write-Host "[+] Ket qua ky so: $($status.Status) - $($status.StatusMessage)" -ForegroundColor Green

    $sig = Get-AuthenticodeSignature -FilePath $ExePath
    Write-Host "[+] Thong tin chu ky Authenticode:" -ForegroundColor Cyan
    $sig | Format-List Status, StatusMessage, Path
    $sig.SignerCertificate | Format-List Subject, Thumbprint, NotAfter
} else {
    Write-Host "[!] Khong tim thay file: $ExePath. Hay build truoc khi ky." -ForegroundColor Yellow
}
