#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"

const char* ssid = "ESP32-Chat";
const char* password = "12345678";

WebServer server(80);
String chatLog = "";

// -------------------- HTML PAGE ----------------------
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Chatroom</title> <!-- Title set to ESP32 Chatroom -->
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <!-- Font Awesome for icons (three lines menu) - Using a robust CDN link -->
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css" crossorigin="anonymous" referrerpolicy="no-referrer" />
  <style>
    /* Ensure consistent box model and full viewport height */
    html, body {
      box-sizing: border-box;
      height: 100%; /* Ensure html and body take full height of the viewport */
      margin: 0;
      padding: 0;
      overflow: hidden; /* Prevent overall page scroll */
      -webkit-text-size-adjust: 100%; /* Prevent text scaling on mobile browsers */
    }
    *, *::before, *::after {
      box-sizing: inherit;
    }

    body {
      font-family: 'Segoe UI', sans-serif;
      background-color: #eae6df; /* Light background for the overall page */
      display: flex; /* Use flexbox to center the chat-container */
      flex-direction: column;
      align-items: center; /* Center chat-container horizontally */
      min-height: 100vh; /* Fallback/ensure full height */
    }
    .chat-container {
      width: 100%;
      height: 100%; /* Make the chat container fill parent height */
      max-width: 600px; /* Max width for larger screens */
      display: flex; /* Make chat-container a flex column */
      flex-direction: column; /* Stack header, chat-box, input-area vertically */
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); /* Subtle shadow for the chat box */
      background-color: #ece5dd; /* Chat background color */
      position: relative; /* Needed for z-index context, though fixed elements are relative to viewport */
    }
    .chat-header {
      background-color: #075e54; /* WhatsApp header green */
      color: white;
      padding: 15px;
      font-size: 1.3em;
      font-weight: bold;
      display: flex; /* Use flexbox for alignment of title and button */
      justify-content: space-between; /* Space out title and action button */
      align-items: center;
      z-index: 10; /* Ensure header stays on top */
      flex-shrink: 0; /* Prevent header from shrinking */
      /* Position fixed to the top of the viewport */
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      width: 100%;
      max-width: 600px; /* Match chat-container max-width */
      margin: 0 auto; /* Center on larger screens */
    }
    .chat-header h1 {
      margin: 0; /* Remove default margin from h1 */
      font-size: 1.3em;
    }
    .header-actions {
      display: flex;
      align-items: center;
    }
    .header-actions button {
      background: none;
      border: none;
      color: white; /* Icon/text color remains white for visibility on dark header */
      font-size: 1.8em; /* Increased size for better visibility of text character */
      cursor: pointer;
      padding: 5px;
      margin-left: 10px;
      outline: none; /* Remove outline on focus */
      -webkit-tap-highlight-color: transparent; /* Remove tap highlight on mobile */
      line-height: 1; /* Adjust line-height for vertical centering of text character */
    }
    #chat-box {
      flex: 1; /* Allows chat box to grow and take all available space */
      overflow-y: auto; /* Enable vertical scrolling for messages */
      padding: 10px;
      background-color: #ece5dd; /* Chat background color */
      display: flex;
      flex-direction: column; /* Messages stack vertically */
      /* Padding to account for fixed header and input area */
      padding-top: 60px; /* Adjusted based on estimated header height */
      padding-bottom: 70px; /* Adjusted based on estimated input area height */
    }
    .message {
      margin: 8px 0;
      padding: 10px 15px;
      max-width: 70%; /* Limit message bubble width */
      border-radius: 8px;
      position: relative;
      word-wrap: break-word; /* Ensure long words break */
      animation: fadeIn 0.4s ease-in-out; /* Smooth fade-in animation */
      box-shadow: 0 1px 0.5px rgba(0, 0, 0, 0.13); /* Subtle shadow like WhatsApp messages */
    }
    .own {
      background-color: #dcf8c6; /* WhatsApp sent message color */
      align-self: flex-end; /* Align to the right */
      border-bottom-right-radius: 0; /* Pointy corner for own message */
    }
    .other {
      background-color: #fff; /* WhatsApp received message color */
      align-self: flex-start; /* Align to the left */
      border-bottom-left-radius: 0; /* Pointy corner for other message */
    }
    .name {
      font-weight: bold;
      color: #075e54; /* Dark green for sender name */
      font-size: 0.9em;
      margin-bottom: 3px;
    }
    .input-area {
      display: flex; /* Use flexbox for input and button alignment */
      align-items: flex-end; /* Align items to the bottom (important for textarea) */
      padding: 8px;
      background: #f0f0f0; /* Light grey background for input area */
      border-top: 1px solid #ccc; /* Separator line */
      z-index: 10; /* Ensure input area is above chat messages */
      flex-shrink: 0; /* Prevent input area from shrinking */
      /* Position fixed to the bottom of the viewport */
      position: fixed;
      bottom: 0;
      left: 0;
      right: 0;
      width: 100%;
      max-width: 600px; /* Match chat-container max-width */
      margin: 0 auto; /* Center on larger screens */
      box-sizing: border-box; /* Include padding in width */
    }
    #name, #msg {
      font-size: 1em;
      padding: 10px 15px; /* Adjusted padding for better WhatsApp match */
      margin: 0; /* Remove default margins */
      border-radius: 20px; /* Highly rounded input fields */
      border: 1px solid #ddd;
      box-sizing: border-box; /* Include padding and border in element's total width */
      outline: none; /* Remove outline on focus */
      resize: none; /* Prevent manual resizing of textarea */
      max-height: 100px; /* Limit textarea height */
      overflow-y: auto; /* Enable scrolling for long messages in textarea */
      -webkit-appearance: none; /* Remove iOS default styling for inputs */
      -moz-appearance: none; /* Remove Firefox default styling for inputs */
      appearance: none; /* Standard property */
    }
    #name {
        width: 100px; /* Fixed width for name input */
        margin-right: 8px; /* Space between name and message input */
        flex-shrink: 0; /* Prevent shrinking */
    }
    #msg {
        flex: 1; /* Message input takes up remaining space */
        margin-right: 8px; /* Space between message input and send button */
        min-height: 45px; /* Minimum height to match button */
        /* Padding adjusted to center text vertically */
        padding-top: 12px; 
        padding-bottom: 12px;
        line-height: 1.2; /* Adjust line height for better spacing */
    }
    .send-btn {
      background-color: #075e54; /* Dark green like header */
      color: white;
      border: none;
      border-radius: 50%; /* Circular button */
      width: 48px; /* Slightly larger for better touch target */
      height: 48px; /* Slightly larger for better touch target */
      display: flex;
      justify-content: center;
      align-items: center;
      cursor: pointer;
      box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2); /* Floating shadow */
      font-size: 0.8em; /* Smaller font size for "Send" text */
      font-weight: bold; /* Make text bold */
      outline: none; /* Remove outline on focus */
      transition: background-color 0.2s ease; /* Smooth transition on hover/active */
      flex-shrink: 0; /* Prevent shrinking */
      -webkit-tap-highlight-color: transparent; /* Remove tap highlight on mobile */
      text-transform: uppercase; /* Optional: Make "Send" uppercase */
    }
    .send-btn:active {
      background-color: #054c44; /* Darker shade of header green when clicked */
    }
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(10px); }
      to { opacity: 1; transform: translateY(0); }
    }
  </style>
</head>
<body>
  <div class="chat-container">
    <div class="chat-header">
      <h1>ESP32 Chatroom</h1> <!-- Title changed to ESP32 Chatroom -->
      <div class="header-actions">
        <!-- Three vertical dots character for clear chat button -->
        <button onclick="clearChat()">&#x22EE;</button>
      </div>
    </div>
    <div id="chat-box"></div>
    <div class="input-area">
      <input type="text" id="name" placeholder="Your name">
      <textarea id="msg" placeholder="Type your message"></textarea>
      <button class="send-btn" onclick="sendMsg()">Send</button> <!-- Changed to text "Send" -->
    </div>
  </div>
  <script>
    let lastLog = "";
    const chatBox = document.getElementById("chat-box");
    const msgInput = document.getElementById("msg");
    const nameInput = document.getElementById("name");

    // Function to fetch and display chat messages
    function fetchChat() {
      fetch("/chat")
        .then(r => r.text())
        .then(t => {
          if (t !== lastLog) {
            lastLog = t;
            const wasAtBottom = chatBox.scrollHeight - chatBox.scrollTop <= chatBox.clientHeight + 1; // Check if user was at bottom
            chatBox.innerHTML = ""; // Clear existing messages
            const lines = t.trim().split("||"); // Split by message delimiter
            lines.forEach(entry => {
              const parts = entry.split("::"); // Split name and message
              if (parts.length < 2) return; // Skip malformed entries
              const name = parts[0];
              const msg = parts[1];
              const msgDiv = document.createElement("div");
              // Determine if it's the user's own message based on the name input
              const isOwnMessage = name === nameInput.value.trim();
              msgDiv.className = "message " + (isOwnMessage ? "own" : "other");
              msgDiv.innerHTML = `<div class="name">${name}</div>${msg}`;
              chatBox.appendChild(msgDiv);
            });
            // Scroll to the bottom only if user was already at the bottom or it's a new message from self
            if (wasAtBottom) {
                chatBox.scrollTop = chatBox.scrollHeight;
            }
          }
        })
        .catch(error => console.error("Error fetching chat:", error)); // Basic error handling
    }

    // Function to send a message
    function sendMsg() {
      const name = nameInput.value.trim();
      const msg = msgInput.value.trim();
      if (!name || !msg) {
        console.log("Name or message cannot be empty.");
        return;
      }
      fetch("/send", {
        method: "POST",
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: `name=${encodeURIComponent(name)}&msg=${encodeURIComponent(msg)}`
      })
      .then(() => {
        msgInput.value = ""; // Clear message input after sending
        msgInput.style.height = 'auto'; // Reset textarea height
        fetchChat(); // Refresh chat log
      })
      .catch(error => console.error("Error sending message:", error)); // Basic error handling
    }

    // Function to clear the chat log
    function clearChat() {
      // Use a confirmation dialog before clearing the chat
      if (confirm("Are you sure you want to clear the entire chat history? This action cannot be undone.")) {
        fetch("/clear", { method: "POST" })
          .then(fetchChat) // Refresh chat log after clearing
          .catch(error => console.error("Error clearing chat:", error)); // Basic error handling
      }
    }

    // Auto-resize textarea based on content
    function autoResizeTextarea() {
        msgInput.style.height = 'auto'; // Reset height to recalculate
        msgInput.style.height = msgInput.scrollHeight + 'px'; // Set height to scrollHeight
    }

    // Event listener for Enter/Shift+Enter
    msgInput.addEventListener("keydown", function(event) {
      if (event.key === "Enter") {
        if (event.shiftKey) {
          // Shift + Enter: new line
          // Allow default behavior
        } else {
          // Enter: send message
          event.preventDefault(); // Prevent default new line
          sendMsg();
        }
      }
    });

    // Event listener for textarea input to auto-resize
    msgInput.addEventListener("input", autoResizeTextarea);


    // Set a default name from local storage or "Guest"
    if (localStorage.getItem("chatName")) {
        nameInput.value = localStorage.getItem("chatName");
    } else {
        nameInput.value = "Guest"; // Default name
    }

    // Save name to local storage whenever the name input changes
    nameInput.addEventListener("input", function() {
        localStorage.setItem("chatName", this.value.trim());
    });

    // Fetch chat messages every 3 seconds
    setInterval(fetchChat, 3000);
    // Fetch chat messages immediately on page load
    fetchChat();
  </script>
</body>
</html>
)rawliteral";

// -------------------- SETUP ----------------------
void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  // Reduce WiFi power save to minimize heat and power consumption
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  Serial.println("ESP32 Chat AP Started");
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());

  // Handle root URL request
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", MAIN_page);
  });

  // Handle chat log request
  server.on("/chat", HTTP_GET, []() {
    server.send(200, "text/plain", chatLog);
  });

  // Handle sending new messages
  server.on("/send", HTTP_POST, []() {
    String name = server.arg("name");
    String msg = server.arg("msg");
    // Basic sanitation: remove delimiters to prevent breaking the log format
    name.replace("::", ""); msg.replace("::", "");
    name.replace("||", ""); msg.replace("||", "");
    
    // Optional: Limit chatLog length to prevent memory overflow on ESP32
    // Adjust the maximum length (e.g., 2048 bytes) based on your ESP32's available RAM
    if (chatLog.length() > 2048) { 
        // Find the first occurrence of "||" and remove messages before it
        int firstDelimiter = chatLog.indexOf("||");
        if (firstDelimiter != -1) {
            chatLog = chatLog.substring(firstDelimiter + 2); 
        } else {
            // If no delimiter found (e.g., very long single message), just clear
            chatLog = "";
        }
    }
    
    // Append new message to the chat log
    chatLog += name + "::" + msg + "||";
    server.send(200, "text/plain", "ok");
  });

  // Handle clearing the chat log
  server.on("/clear", HTTP_POST, []() {
    chatLog = ""; // Clear the chatLog string
    server.send(200, "text/plain", "cleared");
  });

  // Start the web server
  server.begin();
}

// -------------------- LOOP ----------------------
void loop() {
  server.handleClient(); // Handle incoming client requests
  delay(10); // Small delay to reduce CPU usage
}
