param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$OutputPath,
    [Parameter(Mandatory = $true)][string]$FunctionName,
    [int]$Width = 34,
    [switch]$NeonTint,
    [switch]$AlleyBackground
)

Add-Type -AssemblyName System.Drawing

function Get-XtermColor([int]$r, [int]$g, [int]$b) {
    $grayAverage = ($r + $g + $b) / 3
    $graySpread = [Math]::Max($r, [Math]::Max($g, $b)) - [Math]::Min($r, [Math]::Min($g, $b))
    if ($graySpread -lt 14) {
        $gray = [Math]::Max(0, [Math]::Min(23, [Math]::Round(($grayAverage - 8) / 10)))
        return 232 + $gray
    }
    $ri = [Math]::Round($r / 255 * 5)
    $gi = [Math]::Round($g / 255 * 5)
    $bi = [Math]::Round($b / 255 * 5)
    return 16 + 36 * $ri + 6 * $gi + $bi
}

function Get-ScenePixel($bitmap, [int]$x, [int]$y) {
    $pixel = $bitmap.GetPixel($x, $y)
    $r = [int]$pixel.R
    $g = [int]$pixel.G
    $b = [int]$pixel.B
    if ($AlleyBackground -and $r -gt 225 -and $g -gt 225 -and $b -gt 225) {
        $mortar = (($y % 10) -eq 0) -or ((($x + 6 * [Math]::Floor($y / 10)) % 18) -eq 0)
        if ($mortar) { return @(18, 45, 64) }
        $shade = 12 + (($x * 7 + $y * 3) % 12)
        return @($shade, 24 + [Math]::Floor($shade / 3), 38 + [Math]::Floor($shade / 2))
    }
    if ($NeonTint) {
        $v = [int](0.299 * $r + 0.587 * $g + 0.114 * $b)
        $r = [Math]::Min(255, [int](0.82 * $v + [Math]::Max(0, $v - 145) * 0.45))
        $g = [Math]::Min(255, [int](0.72 * $v))
        $b = [Math]::Min(255, [int](1.08 * $v + 20))
    }
    return @($r, $g, $b)
}

$source = [System.Drawing.Bitmap]::FromFile((Resolve-Path -LiteralPath $InputPath))
try {
    $pixelHeight = [Math]::Max(2, [int][Math]::Round($Width * $source.Height / $source.Width))
    if (($pixelHeight % 2) -ne 0) { ++$pixelHeight }
    $scaled = New-Object System.Drawing.Bitmap($Width, $pixelHeight)
    try {
        $graphics = [System.Drawing.Graphics]::FromImage($scaled)
        try {
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            $graphics.DrawImage($source, 0, 0, $Width, $pixelHeight)
        } finally { $graphics.Dispose() }

        $insideWidth = 106
        $leftPad = [Math]::Floor(($insideWidth - $Width) / 2)
        $rightPad = $insideWidth - $Width - $leftPad
        $border = [char]0x2551
        $halfBlock = [char]0x2580
        $lines = [System.Collections.Generic.List[string]]::new()
        $lines.Add("/* Generated from $([System.IO.Path]::GetFileName($InputPath)); rerun tools/generate_ansi_photo.ps1 to update. */")
        $lines.Add("static void $FunctionName(void) {")
        for ($y = 0; $y -lt $pixelHeight; $y += 2) {
            $row = [System.Text.StringBuilder]::new()
            [void]$row.Append('    puts("' + $border)
            [void]$row.Append(' ' * $leftPad)
            $lastTop = -1
            $lastBottom = -1
            for ($x = 0; $x -lt $Width; ++$x) {
                $top = Get-ScenePixel $scaled $x $y
                $bottom = Get-ScenePixel $scaled $x ($y + 1)
                $topCode = Get-XtermColor $top[0] $top[1] $top[2]
                $bottomCode = Get-XtermColor $bottom[0] $bottom[1] $bottom[2]
                if ($topCode -ne $lastTop -or $bottomCode -ne $lastBottom) {
                    [void]$row.Append("\x1b[38;5;$topCode;48;5;$($bottomCode)m")
                    $lastTop = $topCode
                    $lastBottom = $bottomCode
                }
                [void]$row.Append($halfBlock)
            }
            [void]$row.Append('\x1b[0m')
            [void]$row.Append(' ' * $rightPad)
            [void]$row.Append($border + '");')
            $lines.Add($row.ToString())
        }
        $lines.Add('}')
        [System.IO.File]::WriteAllLines(
            [System.IO.Path]::GetFullPath($OutputPath),
            $lines,
            [System.Text.UTF8Encoding]::new($false)
        )
    } finally { $scaled.Dispose() }
} finally { $source.Dispose() }
