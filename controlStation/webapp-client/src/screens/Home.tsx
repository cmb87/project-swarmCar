import React, {useEffect, useState, useRef } from 'react'

// import Amplify from 'aws-amplify'
// https://theskenengineering.com/building-a-react-js-app-with-aws-iot/


import { WebsocketClient } from '../components/WebsocketClient';

import { Joystick, JoystickShape } from 'react-joystick-component';

import testimg from '../assets/testimage.png';
// import demorobot from '../assets/se/robot.png';
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


export default function Home() {


  const canvasRef = useRef<HTMLCanvasElement>(null);
  const [status, setStatus] = useState<string>("offline");
  const [robotId, setRobotId] = useState<string>("1");
  const [joystickStatus, setJoystickStatus] = useState<IJoystickStatus>({x: 0, y:0, type: "stop" ,direction: "IDLE", distance: 0.0});

  const videoStreamer = new WebsocketClient('', 443, "/" )  

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

    // if nothing is specified draw the testimage
    drawImage(testimg);

    // websocket callback
    const updateImage = ( msg:any ) => {
      setStatus("online");
      const reader = new FileReader();  
      reader.onload = () => {  
        drawImage(reader.result as string);
      };  

      try {
        reader.readAsDataURL(msg.data);
      } catch(err) {
        console.log(err);
      }
    }

    videoStreamer.activateStream(updateImage, robotId);

  }, [robotId])

  // ----------------------------------
  useEffect(() => {
  }, [joystickStatus])

  // ----------------------------------
  const publish = async (d:IJoystickStatus) => {

    // Create an ArrayBuffer to hold the binary data
    const buffer = new ArrayBuffer(12); // Assuming 4 bytes 

    // Create a DataView to write into the ArrayBuffer
    const view = new DataView(buffer);

    // Write the values of the struct into the DataView
    view.setInt32(0, float2int(255*d.x),  true);
    view.setInt32(4, float2int(255*d.y),  true);
    view.setInt32(8, 0, true); // true for little-endian

    // Sending buffer
    videoStreamer.send(buffer);

    // send buffer
    setJoystickStatus({x: d.x, y:-d.y, type: d.type , direction: d.direction, distance: 0.0});

  }

  
  // ----------------------------------
  const publishLightOn = async () => {
    //await PubSub.publish(`light_on/${robotId}`, {});
  }

  // ----------------------------------
  const publishLightOff = async () => {
    //await PubSub.publish(`light_off/${robotId}`, {});
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
                stickColor="#800080"

                move={(d:any)=> publish(d)} 
                stop={(d:any)=> publish({x: 0, y:0, type: "stop" , direction: "IDLE", distance: 0.0})}
                baseShape={JoystickShape.Square}
              />

              <button className="bg-purple-800 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded-xl" onClick={()=> publishLightOn()}>
                Lights On
              </button>

              <button className="bg-purple-300 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded-xl" onClick={()=> publishLightOff()}>
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
                  videoStreamer.disconnect();
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


