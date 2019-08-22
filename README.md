# nodeSwitch

nodeSwitch ist eine an der Wand angebrachte Fernbedienung welche die Möglichkeit bietet unterschiedlichste Geräte zu steuern und
gleichzeitig auch Funktionen zur Benachrichtigung von Bewohnern eines Hauses beinhaltet.

nodeSwitch ist darauf ausgerichtet mit QuickSwitch2.0 zu harmonieren.

Das Gerät stellt eine Webseite zum konfigurieren des WLANs und der Verbindung zu QuickSwitch bereit. Von dort lädt es beim start
alle weiteren Einstellungen.

Darüber hinaus existieren alle folgenden REST-Endpunkte:

```
/
/config
/reset
/update
/ntpGetTime
/showFont
/wakeup
/index.css
```

Für die Kommunikation mit QuickSwitch2.0 existieren darüber hinaus noch folgende:
```
POST /action
POST /alert
POST /variable
```

