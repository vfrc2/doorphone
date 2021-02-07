# doorphone

linphone base dorphone

<!-- inject -->
```
Usage: doorphone [OPTION...] [DEST...]
Call sip phones from provided list, wait call ends and exit.

DEST - list of sip phones, which call to (max 10)

Can be run as daemon, when don't call list immediately, wait for incoming calls
and auto answer.
To call dest phones send SIGUSR1 signal to process
Use env for write process pid into file DOORPHONE_PID_FILE=/var/doorphone.pid

  -d, --daemon               Run in daemon mode, do not exit, answer incoming
                             call, call on signal SIGUSR1
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Report bugs to <vfrc29@gmail.com>.
```
<!-- end -->

## Raspberry Pi config

/etc/asound.conf
```
pcm.!default {
    type plug
    slave {
        pcm "hw:1,0"
    }
}

ctl.!default {
    type hw
    card 1
}

```

Test audio:

```sh
# record
arecord -D plughw:CARD=Device,DEV=0 test.wav
#play
aplay -D plughw:CARD=Device,DEV=0 test.wav 
```


## Cisco SAP2102 config

Line 1 Dialplan (<102:@192.168.88.26:5061>)
Line 2 Dialplan (S0 <:@192.168.88.26:5060>)
