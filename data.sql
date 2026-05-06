CREATE DATABASE IF NOT EXISTS iot_data;
USE iot_data;

CREATE TABLE datas (
  id INT AUTO_INCREMENT PRIMARY KEY,
  temperature FLOAT,       -- from DHT11
  humidity FLOAT,          -- from DHT11
  gas INT,                 -- from MQ sensor
  water INT,               -- from water sensor
  light INT,               -- from LDR sensor
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
