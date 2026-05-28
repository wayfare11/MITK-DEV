# Dual MITK Four Views

Standalone Qt/MITK sample that shows two `QmitkStdMultiWidget` panes in one
window. By default it loads:

- `C:\Users\WB-wangyu\Desktop\医学影像数据\liver.nrrd`
- `C:\Users\WB-wangyu\Desktop\医学影像数据\lung1111111.nrrd`

Build:

```powershell
.\build_release.ps1
```

Run:

```powershell
.\run_release.ps1
```

You can also run the generated executable directly after building:

```powershell
.\build\Release\DualMitkFourViews.exe
```

The build step copies MITK Core auto-load modules into `build\MitkCore\Release`
so image readers such as NRRD are available to the standalone executable.

Optional runtime overrides:

```powershell
.\run_release.ps1 "C:\path\to\first.nrrd" "C:\path\to\second.nrrd"
```
