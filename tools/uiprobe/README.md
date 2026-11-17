# uiprobe

Small Win32 helpers for poking a running QtClone app from the outside without
adding test hooks to the app itself.

```bat
python probe.py launch out\wfdesigner.exe --open big.ui   :: prints pid
python probe.py wins <pid>                                 :: visible windows + rects
python probe.py click <pid> 40 14                          :: click client coords
python probe.py click <pid> 60 148 --double
python probe.py click <pid> 600 400 --right
python probe.py type <pid> "line edit"
python probe.py capture out\shots\shot.png 1200 780        :: screen -> png
python probe.py close <pid>                                :: WM_CLOSE
```

`scan.py` clicks down a column and prints the widget class each click hit; it
uses the app's `QTCLONE_CLICK_DEBUG=1` log (`%TEMP%\qtclone-clicks.log`).
`resize.py` nudges the window size by 1px and back to force a full repaint.
`launch2.py` starts an app with both `QTCLONE_CLICK_DEBUG` and
`QTC_PAINT_DEBUG` enabled (paint log goes to `out/paint.log`).

Set `QTCLONE_CLICK_DEBUG=1` (or use `probe.py launch`) to get a red cross on
every press and a log line with the target class, name, geometry and parent.
