# Background-Pi
Projekt zum Anpassen der Hintergrundbeleuchtung an das angezeigte Fernsehbild

Voraussetzung ist die Installation der Bibliothek _raspicam_ :
https://www.uco.es/investiga/grupos/ava/node/40

### Befehl zum Kompilieren:
```
$ g++ <inputfile>.cpp -o <outputfile> -I/usr/local/include/ -lraspicam
```
Für WiringPi ähnlich:
```
$ g++ <inputfile>.cpp -o <outputfile> -I/usr/local/include -L/usr/local/lib -lwiringPi
```
