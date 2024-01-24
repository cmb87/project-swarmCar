import React, { useEffect, useRef } from "react";  
import io from "socket.io-client";  
  
interface WebRTCVideoProps {  
  signalingServerUrl: string;  
}  
  
const WebRTCVideo: React.FC<WebRTCVideoProps> = ({ signalingServerUrl }) => {  
  const remoteVideoRef = useRef<HTMLVideoElement>(null);  
  
  useEffect(() => {  
    const initWebRTC = async () => {  
      if (!remoteVideoRef.current) return;  
  
      const remoteVideo = remoteVideoRef.current;  
      const pc = new RTCPeerConnection();  
  
      pc.ontrack = (event: RTCTrackEvent) => {  
        if (event.track.kind === "video") {  
          remoteVideo.srcObject = event.streams[0];  
        }  
      };  
  
      const offer = await pc.createOffer({ offerToReceiveVideo: true });  
      await pc.setLocalDescription(offer);  
  
      const socket = io(signalingServerUrl);  
      socket.on("connect", () => {  
        console.log("Connected!")
        socket.emit("sdp", { sdp: pc.localDescription?.sdp, type: pc.localDescription?.type });  
      });  
  
      socket.on("sdp", async (data: RTCSessionDescriptionInit) => {  
        if (data.type === "answer") {  
          await pc.setRemoteDescription(new RTCSessionDescription(data));  
        }  
      });  
    };  
  
    initWebRTC();  
  
    return () => {  
      if (remoteVideoRef.current) {  
        remoteVideoRef.current.srcObject = null;  
      }  
    };  
  }, [signalingServerUrl]);  
  
  return <video autoPlay={true} ref={remoteVideoRef} />;  
};  
  
export default WebRTCVideo;