
---

## 🧑‍💻 Example Unity C# script for sending azimuth/distance to Teensy

Save this to something like `unity/TeensySpatialAudio.cs`.  
You will need to adapt COM-port name, baud rate, and how you compute azimuth/distance (depending on your scene).

```csharp
using System.IO.Ports;
using UnityEngine;

public class TeensySpatialAudio : MonoBehaviour
{
    public string portName = "COM3";     // <--- set your serial port
    public int baudRate = 115200;  
    public Transform listener;          // the “ears” (e.g. main camera)
    public Transform soundSource;       // moving sound source
    public int numPositions = 24;       // must match NUM_POSITIONS in hrtf_filters.h
    public float maxDistance = 20f;     // max distance for volume attenuation

    private SerialPort serial;

    void Start()
    {
        serial = new SerialPort(portName, baudRate);
        serial.Open();
        Debug.Log("Opened serial: " + portName);
    }

    void Update()
    {
        if (!serial.IsOpen) return;

        Vector3 dir = soundSource.position - listener.position;
        float distance = dir.magnitude;
        float az = Mathf.Atan2(dir.x, dir.z) * Mathf.Rad2Deg;
        if (az < 0) az += 360f;

        // map azimuth to discreet index [1..numPositions]
        int idx = Mathf.RoundToInt( az / 360f * numPositions );
        if (idx < 1) idx = 1;
        if (idx > numPositions) idx = numPositions;

        int distInt = Mathf.Clamp(Mathf.RoundToInt(distance * 100f), 1, 999);

        string message =
            "1" +                             // play / reset (always 1 = play)
            distInt.ToString("D3") +         // distance as 3 digits
            idx.ToString("D2") +             // HRTF index as 2 digits
            "\n";

        serial.Write(message);
    }

    void OnDestroy()
    {
        if (serial != null && serial.IsOpen)
            serial.Close();
    }
}
