# Minimal C HTTP Server

A simple, multi-threaded HTTP server written in C for learning and experimentation. It serves static `index.html` files and demonstrates basic concurrency using POSIX threads.

This server handles each client connection in a detached thread, responding with `200 OK` for existing files or `404 Not Found` if the file is missing. 
It supports HTTP/1.1 with `Connection: close` headers and ignores `SIGPIPE` to prevent crashes when clients disconnect abruptly. 
This project is ideal for understanding basic web server architecture, socket programming, and concurrency in C.

---

# Minimal C-Based Server

A lightweight TCP server written in C, designed to run on **Linux-based systems**.  
It demonstrates basic socket programming concepts such as socket creation, binding, listening, and handling client connections.

---

## Features

- Multi-threaded: One detached thread per client connection  
- Serves static HTML content from a defined root directory  
- Handles 200 OK and 404 Not Found responses  
- Supports HTTP/1.1 with `Connection: close`  
- Ignores SIGPIPE to prevent crashes on broken connections  

---
