#ImageMedian, Mode, and Mean Processor V1.0
#This program takes all videos from an input video folder and will run median, mode, or mean operations on them, then output them into an output folder.
#Mode takes the longest to run by far, expect 10+ minutes for one video, Mode doesn't seem to be functio
#This program needs cv2, numpy, scipy, tkinter, and pickle. You can install using "Pip install cv2" and "Pip install numpy" etc in the command prompt.
#The file path it takes is designed for windows, you might have to tweak it for Mac OS.

import cv2
import numpy as np
from scipy.stats import mode
import os
import tkinter as tk
from tkinter import filedialog
import pickle


def save_directories():
    directories = {'input_directory': root.file_path, 'output_directory': root.output_path}
    with open('directories.pickle', 'wb') as handle:
        pickle.dump(directories, handle, protocol=pickle.HIGHEST_PROTOCOL)

def select_files():
    root.file_path = filedialog.askdirectory(initialdir = "/", title = "Select Input Directory")
    input_label.config(text="Input Directory: " + root.file_path)
    
  
def select_output():
    root.output_path = filedialog.askdirectory(initialdir = "/", title = "Select Output Directory")
    output_label.config(text="Output Directory: " + root.output_path)
    save_directories()

def calculate_statistics():
    calculate_button.pack_forget()
    video_files = []

    for file in os.listdir(root.file_path):
        if file.endswith(".mp4") or file.endswith(".mov"): #you might have to tweak the file format of videos depending on what you want to use
            video_files.append(file)
        else:
            continue


    video_count = 0
    for video_file in video_files:
        video_count += 1

        video_path = os.path.join(root.file_path,video_file)
        # Open the video file
        video = cv2.VideoCapture(video_path)




        # Initialize a list for the pixel data
        pixel_data = []


        # Get an initial frame to establish size of the video later
        ret, img = video.read()

        # Get dimensions of original frame
        height, width, channels = img.shape

        # Loop through each frame in the video
        while True:
            # Read the next frame
            ret, frame = video.read()
            
            

            if not ret:
                break
            
            
            # Append the pixel data for the current frame to the list
            pixel_data.append(frame)
            

        # Release the video capture
        video.release()

        # stack the pixel data into a 4D array, with frame number, row pixel, column pixel, and rgb value
        np_pixel_data = np.stack(pixel_data, axis=0)

    print(root.file_path + " " + root.output_path)
    if statistic_var.get()== "Median":
        # Find the median of each R, G, and B value for each pixel across all frames
        median_pixels = np.median(np_pixel_data, axis=0)

        # Save the median image
        cv2.imwrite(os.path.join(root.output_path, "median_image %s.jpg" %video_count), median_pixels)
        print("Image Written")

    if statistic_var.get() == "Mean":
        # Find the mean of each R, G, and B value for each pixel across all frames
        mean_pixels = np.mean(np_pixel_data, axis=0)
        # Save the mean image
        cv2.imwrite(os.path.join(root.output_path, "mean_image %s.jpg" %video_count), mean_pixels)
        print("Image Written")
    
    if statistic_var.get() == 'Mode':
        # Find the mode of each R, G, and B value for each pixel across all frames
        mode_pixels = mode(np_pixel_data, axis=0)[0][0]

        # Save the mode image
        cv2.imwrite(os.path.join(root.output_path, "mode_image %s.jpg" %video_count), mode_pixels)
        print("Image Written")
    root.destroy()
    image_written_window = tk.Tk()
    image_written_window.title("Video Image Manipulation")
    label = tk.Label(image_written_window, text="Image Written")
    label.pack()

root = tk.Tk()
root.title("Video Image Manipulation")

input_label = tk.Label(root, text="Input Directory: Not Selected")
input_label.pack()

input_button = tk.Button(root, text="Select Input Directory", command=select_files)
input_button.pack()

output_label = tk.Label(root, text="Output Directory: Not Selected")
output_label.pack()

output_button = tk.Button(root, text="Select Output Directory", command=select_output)
output_button.pack()

statistic_label = tk.Label(root, text="Select a Statistic:")
statistic_label.pack()

statistic_var = tk.StringVar()
statistic_var.set("Mean")

mean_button = tk.Radiobutton(root, text="Mean", variable=statistic_var, value="Mean")
mean_button.pack()

median_button = tk.Radiobutton(root, text="Median", variable=statistic_var, value="Median")
median_button.pack()

mode_button = tk.Radiobutton(root, text="Mode", variable=statistic_var, value="Mode")
mode_button.pack()

calculate_button = tk.Button(root, text="Calculate", command=calculate_statistics)
calculate_button.pack()

if os.path.exists('directories.pickle'):
    with open('directories.pickle', 'rb') as handle:
        directories = pickle.load(handle)
        root.file_path = directories['input_directory']
        root.output_path = directories['output_directory']
        input_label.config(text="Input Directory: " + root.file_path)
        output_label.config(text="Output Directory: " + root.output_path)

root.mainloop()