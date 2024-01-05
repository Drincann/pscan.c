# PScan

PScan is a versatile command-line utility for network administrators, security professionals, and IT enthusiasts. This tool enables efficient and effective scanning of network ports, providing insights into open and closed ports on a given host. PScan is designed with simplicity and usability in mind, making it a practical tool for network diagnostics and security checks.

## Features

Port Range Scanning: Scan a range of ports to quickly understand the network service status.

Individual Port Scanning: Focus on single or specific ports for targeted analysis.

Customizable Timeout: Set a timeout for scanning operations to balance speed and network safety.

Easy-to-use Interface: Straightforward command-line options for hassle-free operation.

## Installation

Clone the repository:

```bash
git clone https://github.com/yourusername/pscan.git
```

Compile the source code (ensure you have a C compiler installed):

```bash
gcc -o pscan pscan.c
```

Run the compiled binary:

```bash
./pscan
```

## Usage

Run PScan with the desired options:

```css
pscan -h <host> [-t <timeout in ms>] [-p <port range or list>]
```

## Options

- **-h**, **--host**: Specify the host IP to scan (required).

- **-t**, **--timeout**: Set timeout in milliseconds (default is 500ms).

- **-p**, **--port**: Specify a single port, a list of ports, or a range (e.g., 80, 8080-8081).

- **--help**: Display help message.

## Examples

Scan ports 22, 80, and 443 on host 192.168.1.1:

```bash
pscan -h 192.168.1.1 -p 22,80,443
```

Scan ports from 80 to 100 with a timeout of 1000ms:

```bash
pscan -h 192.168.1.1 -t 1000 -p 80-100
```
