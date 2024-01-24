
// https://stackoverflow.com/questions/16333790/node-js-quick-file-server-static-files-over-http
// https://github.com/websockets/ws
// https://www.npmjs.com/package/serve-static

const WebSocket = require("ws");
var fs = require('fs');
const http = require('http');
const url = require('url');
const querystring = require('querystring');  

// const finalhandler = require('finalhandler');
// const serveStatic = require('serve-static');
// const path = require('path');  
// const buildPath = path.join(__dirname, 'build');  
// const serve = serveStatic(buildPath);  


const WEBSOCKETPATH = process.env.WEBSOCKETPATH  || '/control';
const SECRETTOKEN = process.env.SECRETTOKEN || 'token';



// =================================
// Web server
const server = http.createServer(function (req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');
  if (req.method == "POST") {
      res.end("Done");
  } else if (req.method == "GET") {
      res.end("Alive");
  }
});


// =================================
// Set up the WebSocket server  
const wss1 = new WebSocket.Server( {noServer: true });  


// Create unique id
wss1.getUniqueID = function () {
  function s4() {
      return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
  }
  return s4() + s4() + s4();
};





// =================================
wss1.on('connection', function connection(ws, request) {

  const parsedUrl = url.parse(request.url);  
  const queryParams = querystring.parse(parsedUrl.query);  

  ws.id = `${wss1.getUniqueID()}-${queryParams.robotid}-${queryParams.type}`

  console.log(`new connection of type ${queryParams.type} ${ws.id} <=> ${queryParams.robotid}`)


  ws.on('message', function message(data, isBinary) {
    wss1.clients.forEach(function each(client) {
      if (
        client !== ws && 
        client.readyState === WebSocket.OPEN &&
        client.id.split('-')[1] === ws.id.split('-')[1] &&
        client.id.split('-')[2] !== ws.id.split('-')[2]
      ) {

        // We broadcast messages to other clients with the same robotId but not the same type
        client.send(data, { binary: isBinary });
      }
    });
  });

  // Websockets
  ws.on('error', console.error);
  ws.on('close', function close() {  console.log(`client ${ws.id} disconnected!`)});
  
});

// =================================
server.on('upgrade', function upgrade(request, socket, head) {

  const parsedUrl = url.parse(request.url);  
  const { pathname } = parsedUrl;  
  const queryParams = querystring.parse(parsedUrl.query);  
  
  if (pathname === WEBSOCKETPATH) {

    const token = queryParams.token;  

    if ( token !== SECRETTOKEN) {
      console.log('InvalidToken:', token);
      socket.destroy();
    }
      
    wss1.handleUpgrade(request, socket, head, function done(ws) {
      wss1.emit('connection', ws, request);
    });

  } else {
    socket.destroy();
  }
});

server.listen({host:"0.0.0.0", port: 8080 });
console.log(`Server started on port 8080! Websocket connection on ${WEBSOCKETPATH}`);

