# Background-Pi
Projekt zum Anpassen der Hintergrundbeleuchtung an das angezeigte Fernsehbild

Voraussetzung ist die Installation der Bibliothek _raspicam_ :
https://www.uco.es/investiga/grupos/ava/node/40

Für die ein stabiles PWM wurde auf _Pi-Blaster_ gewechselt:
https://github.com/sarfata/pi-blaster

### Befehl zum Kompilieren:
```
$ make bkglight
```

Für die Bibliothek Pi-Blaster müssen die Standard-Pins geändert werden:
```
$ sudo pi-blaster --gpio <pin>,<pin>,<pin>
```
Dies wird bei jedem Programmstart ausgeführt.

Für ein flackerfreies Erlebnis müssen im Pi-Blaster-Code pi-blaster.c die Werte SAMPLE_US und CYCLE_TIME_US angepasst werden, entsprechendes Verkleinern beider Werte erhöht die Hz des Signals. (Zum erneuten Kompilieren siehe install-Skript)