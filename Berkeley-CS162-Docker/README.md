# DEPRECATED. 
**PLEASE SEE THE NEW [WORKSPACE](https://github.com/Berkeley-CS162/cs162-workspace) INSTEAD**
# CS 162 Docker-Based Workspace

## Tools you'll need installed
- [Docker Desktop](https://docs.docker.com/desktop/)

**OR**

- [Docker Engine](https://docs.docker.com/engine/) **AND** [Docker Compose](https://docs.docker.com/compose/)

Docker commands will generally have to be run with "sudo", so if you see usage of `docker-compose` in this documentation, you might need to use `sudo docker-compose`.

## Initial setup

```bash
git clone https://github.com/Berkeley-CS162/docker.git
cd docker
docker-compose up
```
Wait until you see `Docker workspace is ready!`. Then you'll need to terminate the current command with `Ctrl+C`.

## Running the workspace

```bash
docker-compose up -d
```

## SSHing into the Workspace
```bash
ssh vagrant@127.0.0.1 -p 16222
```
When prompted for a password, enter: `vagrant`

## Modifying files locally
Local files are stored in `./docker/.workspace`

## Stopping the Workspace
```bash
docker-compose down
```

## Resetting the Workspace
```bash
sudo rm -rf .workspace
```
