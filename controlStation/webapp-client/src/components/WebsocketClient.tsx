
const TOKEN = 'token'
const DEVICETYPE = 'controller'


export class WebsocketClient {
    private endpoint: string
    private port: number
    private hostname: string
    private websocket: WebSocket | null
    public data: any  = undefined;

    constructor(hostname: string, port: number, endpoint: string ){
        this.hostname = hostname;
        this.port = port;
        this.endpoint = endpoint;

        this.websocket = null;
    };

    activateStream(callback:any, robotId: string | number) {
      
        console.log("Connecting to "+this.getServerUrl(robotId));
        this.websocket = new WebSocket(this.getServerUrl(robotId));

        this.websocket.onopen = (event: Event) => { this.send("hello from React")};
        this.websocket.onmessage = callback;
            //this.data = `data:image/jpg;base64,${event.data}`;

    }

    getServerUrl(robotId: string | number){
        return `ws://192.168.128.130:8080/control?token=${TOKEN}&robotid=${robotId}&type=${DEVICETYPE}`
    }


    send(msg:string | ArrayBufferLike | Blob | ArrayBufferView) {
      if (this.websocket !=null && this.websocket.readyState == WebSocket.OPEN) {
        this.websocket.send(msg);
      } 
    }

    
    disconnect(){

      console.log(this.websocket)
      if (this.websocket != null) {
        this.websocket.close();
        console.log("Closing socket....")
      }
    }
}