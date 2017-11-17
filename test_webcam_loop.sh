#ffmpeg -re -f concat -i <(for i in {1..9999}; do printf "file '%s'\n" /home/dilin/Videos/Webcam/output.avi; done) -pix_fmt yuyv422  -f v4l2 /dev/video1

#rm list.txt
#FILE=/home/dilin/Videos/Webcam/output.avi
FILE=/home/dilin/fyp/people_videos/terrace1-c0.avi
#for i in {1..4}; do printf "file '%s'\n" $FILE >> list.txt; done
#ffmpeg -f concat -i list.txt output.mp4
ffmpeg -stream_loop 4 -re -i $FILE -pix_fmt yuyv422  -f v4l2 /dev/video1
