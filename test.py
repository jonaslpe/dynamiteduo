import cv2
import numpy as np
import serial
import threading

# Global variables for intensity and text
intensity = 0
text = "Waiting for data..."

def read_serial():
    global intensity, text
    ser = serial.Serial('COM6', 115200, timeout=1)
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            try:
                serial_value = int(line)
                intensity = serial_value / 200.0
                intensity = max(0, min(intensity, 5))

                # Conversion from serial value to daily hours and minutes
                total_minutes_per_day = serial_value * 60 // 100
                hours_per_day = total_minutes_per_day // 60
                minutes_per_day = total_minutes_per_day % 60
                print(total_minutes_per_day)
                print(minutes_per_day)


                # Conversion to days per year and years in a lifetime
                days_per_year = total_minutes_per_day * 365 // (24 * 60)
                years_in_lifetime = total_minutes_per_day * 365 * 80 // (24 * 60 * 365)
                total_days_per_month = total_minutes_per_day * 30 // (60)

                years_text = ""
                lifetime_text = ""
                day_text = ""
                month_text = ""
                standard_text = ""
                standard_text2 = ""

                if serial_value < 5:
                    standard_text = "\n\nInput your daily phone usage \nby turning the wheel."
                    standard_text2 = "\nAre you unsure? \nCheck your screentime on your phone."
                    text = f"{standard_text}\n {standard_text2}"

                if total_minutes_per_day >= 1:
                    day_text = f"\n\n{hours_per_day:02d} hours and {minutes_per_day:02d} minutes per day \n\n\n"
                    text = f"{day_text}"

                if total_days_per_month >= 1:
                    month_text = f"{total_days_per_month} hours per month\n"

                if days_per_year >= 1:
                    years_text = f"{days_per_year} days per year\n"
                    text = f"{day_text}\n{month_text}\n{years_text}"


                if years_in_lifetime >= 1:
                    lifetime_text = f"{years_in_lifetime} years on your phone in your lifetime"
                    text = f"{day_text}\n{month_text}\n{years_text}\n{lifetime_text}"

                #years_text = f"You spend: {days_per_year} days on your phone per year\n"

                #lifetime_text = f"You will spend: {years_in_lifetime} years on your phone in your lifetime"

                #text = f"{day_text}\n{years_text}\n{lifetime_text}"
            except ValueError:
                text = "Invalid data"


def dissolve_effect(img, intensity):
    step_size = 40
    img_h, img_w = img.shape[:2]
    velocities = np.random.randint(-10, 10, (int(img_h / step_size), int(img_w / step_size), 2)) * intensity
    opacities = np.ones((int(img_h / step_size), int(img_w / step_size)))

    for i in range(int(img_h / step_size)):
        for j in range(int(img_w / step_size)):
            x_start = j * step_size
            y_start = i * step_size
            x_end = x_start + step_size
            y_end = y_start + step_size

            move_x = velocities[i, j, 0]
            move_y = velocities[i, j, 1]

            new_x_start = int(np.clip(x_start + move_x, 0, img_w - step_size))
            new_y_start = int(np.clip(y_start + move_y, 0, img_h - step_size))

            new_x_end = new_x_start + step_size
            new_y_end = new_y_start + step_size

            section = img[y_start:y_end, x_start:x_end].copy()

            img[new_y_start:new_y_end, new_x_start:new_x_end] = section * opacities[i, j]

            if opacities[i, j] > 0.1 * intensity:
                opacities[i, j] -= 0.1 * intensity

    return img


def camera_feed():
    cap2 = cv2.VideoCapture(1)
    window_name = 'Dissolve Effect'
    cv2.namedWindow('Dissolve Effect', cv2.WINDOW_NORMAL)
    fullscreen = False

    while True:
        ret, frame = cap2.read()
        if not ret:
            break

        frame = dissolve_effect(frame, intensity)
        cv2.imshow(window_name, frame)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('g'):  # Bytt til F11 om du ønsker å bruke F11-tasten
            if fullscreen:
                cv2.setWindowProperty('Dissolve Effect', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_NORMAL)
            else:
                cv2.setWindowProperty('Dissolve Effect', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
            fullscreen = not fullscreen

    cap2.release()
    cv2.destroyAllWindows()






def process_frame(frame):
    # Convert the frame to grayscale and detect edges
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    edges = cv2.Canny(blurred, 50, 150)
    edges_inv = cv2.bitwise_not(edges)
    black_screen = np.zeros_like(frame)
    black_screen[edges_inv == 0] = frame[edges_inv == 0]
    return black_screen


def display_video_and_text():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open video capture device.")
        return

    cv2.namedWindow('Processed Video with Text Overlay', cv2.WINDOW_NORMAL)
    fullscreen = False

    # Choose a different font
    font = cv2.FONT_HERSHEY_SIMPLEX # Change font here
    font_scale = 0.7
    font_thickness = 2
    font_color = (0, 255, 0)

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame from video capture.")
            break

        processed_frame = process_frame(frame)

        # Calculate text size to center the overlay text
        text_size = cv2.getTextSize(text, font, font_scale, font_thickness)[0]
        text_x = (processed_frame.shape[1] - text_size[0]) / 2
        text_y = 30

        # Split text into lines for multi-line support
        lines = text.split('\n')
        for i, line in enumerate(lines):
            text_size = cv2.getTextSize(line, font, font_scale, font_thickness)[0]
            text_x = (processed_frame.shape[1] - text_size[0]) / 2  # Recalculate for each line
            cv2.putText(processed_frame, line, (int(text_x), text_y + i * 30), font, font_scale, font_color, font_thickness)

        cv2.imshow('Processed Video with Text Overlay', processed_frame)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('f'):  # Toggle fullscreen with 'f'
            fullscreen = not fullscreen
            if fullscreen:
                cv2.setWindowProperty('Processed Video with Text Overlay', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
            else:
                cv2.setWindowProperty('Processed Video with Text Overlay', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_NORMAL)

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    threading.Thread(target=camera_feed, daemon=True).start()
    threading.Thread(target=read_serial, daemon=True).start()
    display_video_and_text()
