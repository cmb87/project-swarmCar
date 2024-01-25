import React, {useEffect, useState, useRef } from 'react'
import { WebsocketClient } from '../components/WebsocketClient';
import { Joystick, JoystickShape } from 'react-joystick-component';

import testimg from '../assets/testimage.png';
import {InputFieldSelectSlim}  from '../components/InputFieldSlim';



interface IJoystickStatus {
  x: number
  y: number
  type: string | null
  direction: string | null
  distance: number | null
}

function float2int (value:number) {
  return value | 0;
}


const wsStreamer = new WebsocketClient('localhost', 8080, "/control" )  
const wsVideoStreamer = new WebsocketClient('localhost', 8080, "/video" )  


export default function Home() {


  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [status, setStatus] = useState<string>("offline");
  const [robotId, setRobotId] = useState<string>("1");

  const [joystickStatus, setJoystickStatus] = useState<IJoystickStatus>({x: 0, y:0, type: "stop" ,direction: "IDLE", distance: 0.0});
  

  // ----------------------------------
  const drawImage =  (imageSrc: string) => {
    const canvas = canvasRef.current;  
    if (!canvas) return;

    const ctx = canvas.getContext("2d");  

    const image = new Image();  
    image.src = imageSrc; 
    image.onload = () => {  
      // Draw the decoded image on the canvas  
      canvas!.width = image.width;  
      canvas!.height = image.height;  
      ctx!.drawImage(image, 0, 0, image.width, image.height);  
    }
  }

  // ----------------------------------
  useEffect(() => {

    // Disconnect first
    wsStreamer.disconnect();
    wsVideoStreamer.disconnect();

    // if nothing is specified draw the testimage
    drawImage(testimg);

    // websocket callback
    const wsUpdateImageCB = ( msg:any ) => {
      try {
      const reader = new FileReader();  
      reader.onload = () => {  
        drawImage(reader.result as string);
      };  

      try {
        reader.readAsDataURL(msg.data);
      } catch(err) {
        console.log(err);
      }

      } catch (err) {
        console.log("Error in WS CB")
      }
    }

    // Now start the stream
    wsStreamer.activateStream((msg:any)=> console.log(msg), robotId);
    wsVideoStreamer.activateStream(wsUpdateImageCB, robotId);


  }, [robotId])

  // ----------------------------------
  useEffect(() => {}, [joystickStatus])

  // ----------------------------------
  const publish = (d:IJoystickStatus) => {

    const cmds = {
      t: 0,
      x: float2int(255*d.x),
      y: float2int(255*d.y)
    };
    
    // Sending buffer
    wsStreamer.send(JSON.stringify(cmds));

    // send buffer
    setJoystickStatus({x: d.x, y:-d.y, type: d.type , direction: d.direction, distance: 0.0});

  }

  
  // ----------------------------------
  const publishLightOn = async () => {

    wsStreamer.send(JSON.stringify({t: 2,x: 0,y: 0}));

  }

  // ----------------------------------
  const publishLightOff = async () => {
    wsStreamer.send(JSON.stringify({t: 1,x: 0,y: 0}));
  }

  return (
    <div className="xl:w-2/3 md:w-full sm:w-full w-full">

        <h1 className="my-8 text-4xl font-extrabold leading-none tracking-tight text-gray-700 md:text-5xl lg:text-6xl">Robot Unit {robotId}</h1>

        <div className="grid  gap-4 mb-5 md:grid-cols-1 sm:grid-cols-1 xl:grid-cols-2 grid-cols-1">

          {/* Camera Frame */}
          <div className="flex-col rounded-xl border border-gray-500 bg-gray-200 p-5 shadow-xl w-full">

            <h2 className="my-4 text-xl font-extrabold leading-none tracking-tight text-gray-700 ">Video Stream</h2>
            { status === "online" ? <span className={`bg-green-900 text-white text-xs font-medium mr-1 px-1.5 py-0.5 my-3 rounded`}>{status}</span> : 
              <span className={`bg-red-900 text-white text-xs font-medium mr-1 px-1.5 py-0.5 my-3 rounded`}>{status}</span> 
            }

              <canvas width={"100%"} height={"100%"} ref={canvasRef} className='w-full rounded-xl mb-5'></canvas>

              <h2 className="my-4 text-xl font-extrabold leading-none tracking-tight text-gray-700 ">Controller</h2>

              <div className='flex flex-row gap-5'>
              <Joystick 
                size={100}
                sticky={false}
                baseColor="gray"
                throttle={40}
                minDistance={10}
                stickColor="#12086F"

                move={(d:any) => publish(d)} 
                stop={(d:any)=>  publish({x: 0, y:0, type: "stop" , direction: "IDLE", distance: 0.0})}
                baseShape={JoystickShape.Square}
              />

              <button className="bg-blue-800 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded-xl" onClick={()=> publishLightOn()}>
                Lights On
              </button>

              <button className="bg-blue-300 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded-xl" onClick={()=> publishLightOff()}>
                Lights Off
              </button>

              <InputFieldSelectSlim
                id={"X"}
                label={'Robot Unit'}
                options={[
                  {key: "Robot 1", value: "1"},
                  {key: "Robot 2", value: "2"},
                  {key: "Robot 3", value: "3"},
                
                ]}
                onChange={(x:any) => {
                  wsStreamer.disconnect();
                  setRobotId(x);
                }}
                value={robotId}
              />


              </div>

          </div>

          {/* Status */}
          <div className="flex-col rounded-xl border border-gray-500 p-5 shadow-xl w-full">
            
            <h2 className="my-4 text-xl font-extrabold leading-none tracking-tight text-gray-700 ">Status</h2>

            <span className={`bg-red-900 text-white text-xs font-medium mr-1 px-1.5 py-0.5 my-3 rounded`}>x: {joystickStatus.x}</span> 
            <span className={`bg-red-900 text-white text-xs font-medium mr-1 px-1.5 py-0.5 my-3 rounded`}>y: {joystickStatus.y}</span> 
            <span className={`bg-red-900 text-white text-xs font-medium mr-1 px-1.5 py-0.5 my-3 rounded`}>{joystickStatus.direction}</span> 
            <span className={`bg-red-900 text-white text-xs font-medium mr-1 px-1.5 py-0.5 my-3 rounded`}>{joystickStatus.type}</span> 
            <div className="grid grid-cols-2 gap-4 mb-5">

            {/* <img src={demorobot} alt="Demo Robot" className='w-full rounded-xl mt-8 mx-10' /> */}


            </div>    

          </div>
          
        </div>


    </div>
  )
}


