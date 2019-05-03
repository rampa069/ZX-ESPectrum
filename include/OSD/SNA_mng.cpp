// Change running snapshot
void changeSna(String sna_filename) {
    stopULA();
    osdCenteredMsg((String)MSG_LOADING + ": " + sna_filename, LEVEL_INFO);
    cfg_ram_file = "/sna/" + sna_filename;
    cfg_mode_sna = true;
    zx_reset();
    load_ram(cfg_ram_file);
    osdCenteredMsg(MSG_SAVE_CONFIG, LEVEL_WARN);
    config_save();
    startULA();
}
