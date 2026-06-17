#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
TEMPLATE_DIR = os.path.join(BASE_DIR, "template")

def init_project(args):
    project_name = args.name
    if os.path.exists(project_name):
        print(f"Error: Directory {project_name} already exists.")
        return
    
    print(f"Initializing micro-ROS project: {project_name}...")
    shutil.copytree(TEMPLATE_DIR, project_name)
    print(f"Project {project_name} created successfully.")

def build_project(args):
    print("Building project...")
    subprocess.run(["idf.py", "build"], check=True)

def flash_project(args):
    port = args.port or "/dev/ttyUSB0"
    print(f"Flashing project to {port}...")
    subprocess.run(["idf.py", "-p", port, "flash", "monitor"], check=True)

def start_agent(args):
    print("Starting micro-ROS Agent (UDP 8888)...")
    # Using Docker as default for simplicity, can be changed to native
    cmd = [
        "docker", "run", "-it", "--rm", "--net=host",
        "microros/micro-ros-agent:humble", "udp4", "--port", "8888"
    ]
    subprocess.run(cmd)

def sim_project(args):
    print("Starting Wokwi Simulation...")
    if not os.path.exists("wokwi.toml"):
        print("Error: wokwi.toml not found. Are you in a project directory?")
        return
    subprocess.run(["wokwi-cli", "."], check=True)

def main():
    parser = argparse.ArgumentParser(description="micro-ROS ESP32 Development CLI")
    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # init
    parser_init = subparsers.add_parser("init", help="Initialize a new project")
    parser_init.add_argument("name", help="Name of the project")

    # build
    subparsers.add_parser("build", help="Build the project")

    # flash
    parser_flash = subparsers.add_parser("flash", help="Flash the project")
    parser_flash.add_argument("--port", "-p", help="Serial port (default: /dev/ttyUSB0)")

    # agent
    subparsers.add_parser("agent", help="Start micro-ROS agent")

    # sim
    subparsers.add_parser("sim", help="Start Wokwi simulation")

    args = parser.parse_args()

    if args.command == "init":
        init_project(args)
    elif args.command == "build":
        build_project(args)
    elif args.command == "flash":
        flash_project(args)
    elif args.command == "agent":
        start_agent(args)
    elif args.command == "sim":
        sim_project(args)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
