
const TOKEN = 'token'
const DEVICETYPE = 'controller'


export class WebsocketClient {
    private endpoint: string
    private port: number
    private hostname: string
    public ws: WebSocket | null

    // ---------------------------------------------------
    constructor(hostname: string, port: number, endpoint: string ){
        this.hostname = hostname;
        this.port = port;
        this.endpoint = endpoint;
        this.ws = null;
    };

    // ---------------------------------------------------
    activateStream(callback:any, robotId: string | number) {
        console.log("Connecting to "+this.getServerUrl(robotId));

        this.ws = new WebSocket(this.getServerUrl(robotId));

        this.ws.onopen = (event: Event) => { this.send("hello from React")};
        this.ws.onmessage = callback;
        this.ws.onerror = (event: Event) => console.log(event);
    }

    // ---------------------------------------------------
    getServerUrl(robotId: string | number){
        return `ws://${this.hostname}:${this.port}${this.endpoint}?token=${TOKEN}&robotid=${robotId}&type=${DEVICETYPE}`
    }

    // ---------------------------------------------------
    send(msg:any) {
      
      if (this.ws !=null && this.ws.readyState == WebSocket.OPEN) {
        this.ws.send(msg);
        
      } 

      console.log(this.ws !=null)
    }

    // ---------------------------------------------------
    disconnect(){

      console.log(this.ws)
      if (this.ws != null) {
        this.ws.close();
        console.log("Closing socket....")
      }
    }
}