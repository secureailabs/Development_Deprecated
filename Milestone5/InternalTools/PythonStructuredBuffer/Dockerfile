# syntax=docker/dockerfile:1

FROM ubuntu:latest

WORKDIR /app

RUN apt-get update && apt-get install -y python3-pip

COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
RUN apt-get install -y libcurl4-openssl-dev
# COPY . .

# CMD [ "python3", "-m" , "flask", "run", "--host=0.0.0.0"]
CMD ["bash"]
# python3 -m flask run --host=0.0.0.0