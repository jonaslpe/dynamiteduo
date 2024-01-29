import cv2
import numpy as np
import serial
import pygame
import threading



# Global variabel for intensitet og tekst
intensity = 0
text = "Venter på data..."


# Trådfunksjon for å lese fra serialporten
def read_serial():
    global intensity, text
    ser = serial.Serial('COM5', 115200, timeout=1)
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            try:
                serial_value = int(line)
                intensity = serial_value / 200.0
                intensity = max(0, min(intensity, 5))

                # Omregning fra serial-verdi til timer og minutter per dag
                total_minutes_per_day = serial_value * 60 // 100
                hours_per_day = total_minutes_per_day // 60
                minutes_per_day = total_minutes_per_day % 60
                day_text = f"Du bruker: {hours_per_day:02d} timer og {minutes_per_day:02d} minutter på mobilen per dag"

                # Timer og minutter per uke
                total_minutes_per_week = total_minutes_per_day * 7
                hours_per_week = total_minutes_per_week // 60
                minutes_per_week = total_minutes_per_week % 60
                week_text = f"Du bruker: {hours_per_week:02d} timer og {minutes_per_week:02d} minutter på mobilen per uke"

                # Antall dager per måned og per år
                days_per_month = total_minutes_per_day * 30 // (24 * 60)
                days_per_year = total_minutes_per_day * 365 // (24 * 60)
                months_text = f"Du bruker: {days_per_month} dager på mobilen per måned"
                years_text = f"Du bruker: {days_per_year} dager på mobilen per år"

                # Antall år i et 80 år langt liv
                years_in_lifetime = total_minutes_per_day * 365 * 80 // (24 * 60 * 365)
                lifetime_text = f"Du vil bruke: {years_in_lifetime} år på mobilen i løpet av livet ditt"

                text = f"{day_text}\n\n\n{week_text}\n\n\n{months_text}\n\n\n{years_text}\n\n\n{lifetime_text}"
                print(serial_value)
            except ValueError:
                text = "Invalid data"
                continue


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
    cap = cv2.VideoCapture(0)
    window_name = 'Dissolve Effect'
    cv2.namedWindow('Dissolve Effect', cv2.WINDOW_NORMAL)
    fullscreen = False

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        frame = dissolve_effect(frame, intensity)
        cv2.imshow(window_name, frame)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('f'):  # Bytt til F11 om du ønsker å bruke F11-tasten
            if fullscreen:
                cv2.setWindowProperty('Dissolve Effect', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_NORMAL)
            else:
                cv2.setWindowProperty('Dissolve Effect', cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)
            fullscreen = not fullscreen

    cap.release()
    cv2.destroyAllWindows()

def display_serial_data():
    global text
    pygame.init()
    window_size = (800, 600)
    window = pygame.display.set_mode(window_size)
    pygame.display.set_caption("Serial Data Display")
    font = pygame.font.Font(None, 65)  # Juster fontstørrelsen om nødvendig
    fullscreen = False

    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_F11:
                    fullscreen = not fullscreen
                    if fullscreen:
                        window = pygame.display.set_mode((0, 0), pygame.FULLSCREEN)
                    else:
                        window = pygame.display.set_mode(window_size)

        window.fill((0, 0, 0))

        # Henter nåværende vindusstørrelse for korrekt sentrering av tekst
        current_window_size = window.get_size()

        # Deler teksten i linjer og justerer startpunktet for den første linjen
        lines = text.split('\n')
        start_y = current_window_size[1] // 5  # Start en fjerdedel ned på skjermen
        line_height = font.get_linesize()

        for i, line in enumerate(lines):
            rendered_text = font.render(line, True, (255, 255, 255))
            text_rect = rendered_text.get_rect(center=(current_window_size[0] // 2, start_y + i * line_height))
            window.blit(rendered_text, text_rect)

        pygame.display.flip()

    pygame.quit()




if __name__ == "__main__":
    threading.Thread(target=camera_feed, daemon=True).start()
    threading.Thread(target=read_serial, daemon=True).start()

    display_serial_data()