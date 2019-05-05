// Change running snapshot
void changeSna(String sna_filename) {
    stopULA();
    osdCenteredMsg((String)MSG_LOADING + ": " + sna_filename, LEVEL_INFO);
    zx_reset();
    Serial.printf("Loading sna: %s\n", sna_filename.c_str());
    load_ram_128((String)DISK_SNA_DIR + "/" + sna_filename);
    osdCenteredMsg(MSG_SAVE_CONFIG, LEVEL_WARN);
    cfg_ram_file = sna_filename;
    config_save();
    startULA();
}
