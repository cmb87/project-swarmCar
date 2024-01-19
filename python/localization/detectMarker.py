import cv2
import cv2.aruco as aruco
import numpy as np




def detect_and_estimate_poses(video_path, marker_size=6):

    # Create ARUCO dictionary
    aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
    
    # Create ARUCO parameters
    aruco_params = cv2.aruco.DetectorParameters()

    # Open video capture
    cap = cv2.VideoCapture(video_path)

    detector = cv2.aruco.ArucoDetector(aruco_dict, aruco_params)

    while cap.isOpened():
        ret, frame = cap.read()

        if not ret:
            print("End of video or error.")
            break

        # Convert the frame to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        # Detect ArUco markers
        corners, ids, _ = detector.detectMarkers(frame)

        if ids is not None:
            # Draw detected markers and annotate with IDs
            frame = aruco.drawDetectedMarkers(frame, corners, ids)
            

            # # Estimate poses of detected markers
            # cv2.aruco
            # detector.
            # rvecs, tvecs, _ = aruco.estimatePoseSingleMarkers(corners, marker_size, cameraMatrix, distCoeffs)

            # # Draw the pose information on the frame
            # for i in range(len(ids)):
            #     aruco.drawAxis(frame, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1)

        # Display the resulting frame
        cv2.imshow('ArUco Marker Detection and Pose Estimation', frame)

        # Break the loop if 'q' key is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release video capture and close windows
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    # Replace 'your_video_file.mp4' with the path to your video file
    video_path = 0

    # Camera matrix and distortion coefficients (you may need to calibrate your camera for accurate results)
    cameraMatrix = np.array([[1000, 0, 640],
                             [0, 1000, 360],
                             [0, 0, 1]], dtype=np.float64)
    distCoeffs = np.zeros((5, 1), dtype=np.float64)

    detect_and_estimate_poses(video_path)