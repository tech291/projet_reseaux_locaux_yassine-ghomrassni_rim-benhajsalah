# projet_reseaux_locaux_yassine-ghomrassni_rim-benhajsalah
# 🌡️ Système de Monitoring Environnemental IoT

**STM32L475E-IOT01A + MQTT + Node-RED + MongoDB**

> Projet de monitoring temps réel de paramètres environnementaux (température, humidité, pression) avec transmission MQTT, visualisation Node-RED et archivage MongoDB.

---

## 📋 Vue d'ensemble

Ce projet propose **deux versions** du firmware pour la carte **STM32L475E-IOT01A** :

### phase 1 : STM32CubeIDE (UART/PuTTY)
Code C/HAL qui permet de :
- ✅ Lire les capteurs **HTS221** (température, humidité) et **LPS22HB** (pression)
- ✅ Afficher les valeurs sur **PuTTY** via connexion **UART** (115200 baud)

**Fichier :** `main.c` (dans `/CubeIDE/`)

### phase2 : Arduino IDE (WiFi + MQTT)
Code Arduino C++ qui permet de :
- ✅ Lire les mêmes capteurs **HTS221** et **LPS22HB**
- ✅ Se connecter au WiFi via le module **ISM43362**
- ✅ Publier les données vers un broker **MQTT**

**Fichier :** `stm32_mqtt_sensors.ino` (dans `/Arduino/`)
### phase3 :
Les données MQTT sont ensuite :
- 📊 **Visualisées en temps réel** sur un dashboard Node-RED
- 💾 **Archivées** dans une base de données MongoDB

---

## 🏗️ Architecture

### Version 1 (CubeIDE) - Test Basique
```
STM32 (Capteurs) → UART → PuTTY/Tera Term
```

### Version 2 (Arduino) - IoT Complet
```
STM32 (Capteurs) → WiFi → MQTT Broker → Node-RED Dashboard
                                      ↓
                                   MongoDB
```

**Topics MQTT utilisés :**
- `/iot/yassin_rym/temperature`
- `/iot/yassin_rym/humidite`
- `/iot/yassin_rym/pression`

---

## 🛠️ Matériel requis

- Carte **STM32L475E-IOT01A**
- Câble USB (Micro-USB)
- Réseau WiFi **2.4 GHz** (⚠️ 5 GHz non supporté)

---







### 3. MQTT Broker (MQTT tools)
![BsaL2oCY](https://github.com/user-attachments/assets/8bc9b048-d030-4e9a-8e15-dbd23a8236c0)



### 4. Node-RED

![TqURlcpY](https://github.com/user-attachments/assets/52f8ef31-d32f-457a-b6e1-968f29168c37)
![fxEX65ht](https://github.com/user-attachments/assets/bf6c7d9f-78b4-4381-86ca-91e6353d0c00)



### 5. MongoDB
![NCVnc9IV](https://github.com/user-attachments/assets/4801cde7-9edd-44cc-9459-8779dda8bba5)

---

### ⚙️ Configuration

### Dans le code Arduino :
```cpp
// WiFi
char ssid[] = "VOTRE_WIFI";
char pass[] = "VOTRE_MOT_DE_PASSE";

// MQTT (Cloud public)
const char* mqtt_server = "broker.hivemq.com";

// OU MQTT (Local)
const char* mqtt_server = "192.168.X.X";  // IP de votre PC

// Topics (personnalisez)
const char* topic_temp = "/iot/VOTRE_NOM/temperature";
const char* topic_hum = "/iot/VOTRE_NOM/humidite";
const char* topic_press = "/iot/VOTRE_NOM/pression";
```

---

## 🚀 Utilisation

### Version 1 : CubeIDE (Test Capteurs)

1. **Compiler et flasher** dans STM32CubeIDE
2. **Ouvrir PuTTY/Tera Term** : COM port, 115200 baud
3. **Observer les mesures** toutes les 2 secondes
4. **LED PB14** clignote à chaque lecture

**Sortie attendue :**
```
*** STM32 UART TEST - HELLO WORLD ***

===========================================
  STM32L475E-IOT01A - Lecture des capteurs
===========================================

Initialisation des capteurs...
- Capteur de temperature : OK
- Capteur d'humidite : OK
- Capteur de pression : OK

Demarrage des mesures...

┌─────────────────────────────────────┐
│      MESURES DES CAPTEURS           │
├─────────────────────────────────────┤
│ Temperature  :  24.50 °C            │
│ Humidite     :  53.20 %             │
│ Pression     : 1010.58 hPa          │
└─────────────────────────────────────┘
```

---

### Version 2 : Arduino (IoT Complet)


### 2. Monitoring
**Serial Monitor (115200 baud) :**
```
✓ WiFi Connecte! IP: 192.168.43.27
Connexion MQTT... OK!
┌─────────────────────────────────┐
│ Temperature: 24.50 °C           │
│ Humidite   : 53.20 %            │
│ Pression   : 1010.58 hPa        │
└─────────────────────────────────┘
✓ Toutes les valeurs publiees!
```






## 👥 Auteurs

**Yassin & Rym**  
Projet IoT - Janvier 2026

**Évolution du projet :**
1. **Phase 1** : Test capteurs avec STM32CubeIDE + UART
2. **Phase 2** : Intégration WiFi + MQTT avec Arduino
3. **Phase 3** : Dashboard Node-RED + Base MongoDB

---
