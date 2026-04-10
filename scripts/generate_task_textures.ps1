Add-Type -AssemblyName System.Drawing

$texDir = "C:\Users\tamim\Desktop\Graphics_Project\textures"
if (!(Test-Path $texDir)) { New-Item -ItemType Directory -Path $texDir | Out-Null }

function New-TaskTexture {
    param(
        [string]$Path,
        [string]$Title,
        [string]$Subtitle,
        [string]$BgHex
    )

    $bmp = New-Object System.Drawing.Bitmap 512, 256
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit

    $bgColor = [System.Drawing.ColorTranslator]::FromHtml($BgHex)
    $g.Clear($bgColor)

    $frameBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(220,255,255,255))
    $g.FillRectangle($frameBrush, 14, 14, 484, 228)

    $innerBrush = New-Object System.Drawing.SolidBrush ($bgColor)
    $g.FillRectangle($innerBrush, 24, 24, 464, 208)

    $titleFont = New-Object System.Drawing.Font('Arial', 36, [System.Drawing.FontStyle]::Bold)
    $subFont = New-Object System.Drawing.Font('Arial', 18, [System.Drawing.FontStyle]::Regular)

    $white = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(245,245,245))
    $line = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(180,255,255,255), 3)

    $g.DrawString($Title, $titleFont, $white, 36, 62)
    $g.DrawLine($line, 36, 126, 476, 126)
    $g.DrawString($Subtitle, $subFont, $white, 36, 146)

    $bmp.Save($Path, [System.Drawing.Imaging.ImageFormat]::Jpeg)

    $line.Dispose(); $white.Dispose(); $subFont.Dispose(); $titleFont.Dispose()
    $innerBrush.Dispose(); $frameBrush.Dispose(); $g.Dispose(); $bmp.Dispose()
}

New-TaskTexture -Path (Join-Path $texDir 'task_stamping.jpg')   -Title 'STAMPING'   -Subtitle 'Press & Forming Complete'    -BgHex '#355C7D'
New-TaskTexture -Path (Join-Path $texDir 'task_welding.jpg')    -Title 'WELDING'    -Subtitle 'Joint Fusion In Progress'   -BgHex '#C06C84'
New-TaskTexture -Path (Join-Path $texDir 'task_inspection.jpg') -Title 'INSPECTION' -Subtitle 'Vision QA Validation'       -BgHex '#6C5B7B'
New-TaskTexture -Path (Join-Path $texDir 'task_packing.jpg')    -Title 'PACKING'    -Subtitle 'Carton + Palletizing'       -BgHex '#2A9D8F'
New-TaskTexture -Path (Join-Path $texDir 'task_shipping.jpg')   -Title 'SHIPPING'   -Subtitle 'Wrapped Goods Ready To Ship' -BgHex '#264653'
