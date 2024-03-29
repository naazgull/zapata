/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or distribute
  this software, either in source code form or as a compiled binary, for any
  purpose, commercial or non-commercial, and by any means.

  In jurisdictions that recognize copyright laws, the author or authors of this
  software dedicate any and all copyright interest in the software to the public
  domain. We make this dedication for the benefit of the public at large and to
  the detriment of our heirs and successors. We intend this dedication to be an
  overt act of relinquishment in perpetuity of all present and future rights to
  this software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <dirent.h>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <zapata/exceptions/SyntaxErrorException.h>

namespace zpt {

extern const char* mimetype_names[];
extern const char* mimetype_extensions[];

enum MIMEType {
    x_world_x_3dmf,
    application_octet_stream,
    application_x_authorware_bin,
    application_x_authorware_map,
    application_x_authorware_seg,
    text_vnd_abc,
    text_html,
    video_animaflex,
    application_postscript,
    audio_aiff,
    audio_x_aiff,
    application_x_aim,
    text_x_audiosoft_intra,
    application_x_navi_animation,
    application_x_nokia_9000_communicator_add_on_software,
    application_mime,
    application_arj,
    image_x_jg,
    video_x_ms_asf,
    text_x_asm,
    text_asp,
    application_x_mplayer2,
    video_x_ms_asf_plugin,
    audio_basic,
    audio_x_au,
    application_x_troff_msvideo,
    video_avi,
    video_msvideo,
    video_x_msvideo,
    video_avs_video,
    application_x_bcpio,
    application_mac_binary,
    application_macbinary,
    application_x_binary,
    application_x_macbinary,
    image_bmp,
    image_x_windows_bmp,
    application_book,
    application_x_bzip2,
    application_x_bsh,
    application_x_bzip,
    text_plain,
    text_x_c,
    application_vnd_ms_pki_seccat,
    application_clariscad,
    application_x_cocoa,
    application_cdf,
    application_x_cdf,
    application_x_netcdf,
    application_pkix_cert,
    application_x_x509_ca_cert,
    application_x_chat,
    application_java,
    application_java_byte_code,
    application_x_java_class,
    application_x_cpio,
    application_mac_compactpro,
    application_x_compactpro,
    application_x_cpt,
    application_pkcs_crl,
    application_pkix_crl,
    application_x_x509_user_cert,
    application_x_csh,
    text_x_script_csh,
    application_x_pointplus,
    text_css,
    application_x_director,
    application_x_deepv,
    video_x_dv,
    video_dl,
    video_x_dl,
    application_msword,
    application_commonground,
    application_drafting,
    application_x_dvi,
    drawing_x_dwf,
    model_vnd_dwf,
    application_acad,
    image_vnd_dwg,
    image_x_dwg,
    application_dxf,
    text_x_script_elisp,
    application_x_bytecode_elisp,
    application_x_elc,
    application_x_envoy,
    application_x_esrehber,
    text_x_setext,
    application_envoy,
    text_x_fortran,
    application_vnd_fdf,
    application_fractals,
    image_fif,
    video_fli,
    video_x_fli,
    image_florian,
    text_vnd_fmi_flexstor,
    video_x_atomic3d_feature,
    image_vnd_fpx,
    image_vnd_net_fpx,
    application_freeloader,
    audio_make,
    image_g3fax,
    image_gif,
    video_gl,
    video_x_gl,
    audio_x_gsm,
    application_x_gsp,
    application_x_gss,
    application_x_gtar,
    application_x_compressed,
    application_x_gzip,
    multipart_x_gzip,
    text_x_h,
    application_x_hdf,
    application_x_helpfile,
    application_vnd_hp_hpgl,
    text_x_script,
    application_hlp,
    application_x_winhelp,
    application_binhex,
    application_binhex4,
    application_mac_binhex,
    application_mac_binhex40,
    application_x_binhex40,
    application_x_mac_binhex40,
    application_hta,
    text_x_component,
    text_webviewhtml,
    x_conference_x_cooltalk,
    image_x_icon,
    image_ief,
    application_iges,
    model_iges,
    application_x_ima,
    application_x_httpd_imap,
    application_inf,
    application_x_internett_signup,
    application_x_ip2,
    video_x_isvideo,
    audio_it,
    application_x_inventor,
    i_world_i_vrml,
    application_x_livescreen,
    audio_x_jam,
    text_x_java_source,
    application_x_java_commerce,
    image_jpeg,
    image_pjpeg,
    image_x_jps,
    application_x_javascript,
    image_jutvision,
    audio_midi,
    music_x_karaoke,
    application_x_ksh,
    text_x_script_ksh,
    audio_nspaudio,
    audio_x_nspaudio,
    audio_x_liveaudio,
    application_x_latex,
    application_lha,
    application_x_lha,
    application_x_lisp,
    text_x_script_lisp,
    text_x_la_asf,
    application_x_lzh,
    application_lzx,
    application_x_lzx,
    text_x_m,
    video_mpeg,
    audio_mpeg,
    audio_x_mpequrl,
    application_x_troff_man,
    application_x_navimap,
    application_mbedlet,
    application_x_magic_cap_package_1_0,
    application_mcad,
    application_x_mathcad,
    image_vasa,
    text_mcf,
    application_netmc,
    application_x_troff_me,
    message_rfc822,
    application_x_midi,
    audio_x_mid,
    audio_x_midi,
    music_crescendo,
    x_music_x_midi,
    application_x_frame,
    application_x_mif,
    www_mime,
    audio_x_vnd_audioexplosion_mjuicemediafile,
    video_x_motion_jpeg,
    application_base64,
    application_x_meme,
    audio_mod,
    audio_x_mod,
    video_quicktime,
    video_x_sgi_movie,
    audio_x_mpeg,
    video_x_mpeg,
    video_x_mpeq2a,
    audio_mpeg3,
    audio_x_mpeg_3,
    application_x_project,
    application_vnd_ms_project,
    application_marc,
    application_x_troff_ms,
    application_x_vnd_audioexplosion_mzz,
    image_naplps,
    application_vnd_nokia_configuration_message,
    image_x_niff,
    application_x_mix_transfer,
    application_x_conference,
    application_x_navidoc,
    application_oda,
    application_x_omc,
    application_x_omcdatamaker,
    application_x_omcregerator,
    text_x_pascal,
    application_pkcs10,
    application_x_pkcs10,
    application_pkcs_12,
    application_x_pkcs12,
    application_x_pkcs7_signature,
    application_pkcs7_mime,
    application_x_pkcs7_mime,
    application_x_pkcs7_certreqresp,
    application_pkcs7_signature,
    application_pro_eng,
    text_pascal,
    image_x_portable_bitmap,
    application_vnd_hp_pcl,
    application_x_pcl,
    image_x_pict,
    image_x_pcx,
    chemical_x_pdb,
    application_pdf,
    audio_make_my_funk,
    image_x_portable_graymap,
    image_x_portable_greymap,
    image_pict,
    application_x_newton_compatible_pkg,
    application_vnd_ms_pki_pko,
    text_x_script_perl,
    application_x_pixclscript,
    image_x_xpixmap,
    text_x_script_perl_module,
    application_x_pagemaker,
    image_png,
    application_x_portable_anymap,
    image_x_portable_anymap,
    application_mspowerpoint,
    application_vnd_ms_powerpoint,
    model_x_pov,
    image_x_portable_pixmap,
    application_powerpoint,
    application_x_mspowerpoint,
    application_x_freelance,
    paleovu_x_pv,
    text_x_script_phyton,
    applicaiton_x_bytecode_python,
    audio_vnd_qcelp,
    image_x_quicktime,
    video_x_qtc,
    audio_x_pn_realaudio,
    audio_x_pn_realaudio_plugin,
    audio_x_realaudio,
    application_x_cmu_raster,
    image_cmu_raster,
    image_x_cmu_raster,
    text_x_script_rexx,
    image_vnd_rn_realflash,
    image_x_rgb,
    application_vnd_rn_realmedia,
    audio_mid,
    application_ringing_tones,
    application_vnd_nokia_ringing_tone,
    application_vnd_rn_realplayer,
    application_x_troff,
    image_vnd_rn_realpix,
    text_richtext,
    text_vnd_rn_realtext,
    application_rtf,
    application_x_rtf,
    video_vnd_rn_realvideo,
    audio_s3m,
    application_x_tbook,
    application_x_lotusscreencam,
    text_x_script_guile,
    text_x_script_scheme,
    video_x_scm,
    application_sdp,
    application_x_sdp,
    application_sounder,
    application_sea,
    application_x_sea,
    application_set,
    text_sgml,
    text_x_sgml,
    application_x_sh,
    application_x_shar,
    text_x_script_sh,
    text_x_server_parsed_html,
    audio_x_psid,
    application_x_sit,
    application_x_stuffit,
    application_x_koan,
    application_x_seelogo,
    application_smil,
    audio_x_adpcm,
    application_solids,
    application_x_pkcs7_certificates,
    text_x_speech,
    application_futuresplash,
    application_x_sprite,
    application_x_wais_source,
    application_streamingmedia,
    application_vnd_ms_pki_certstore,
    application_step,
    application_sla,
    application_vnd_ms_pki_stl,
    application_x_navistyle,
    application_x_sv4cpio,
    application_x_sv4crc,
    application_x_world,
    x_world_x_svr,
    application_x_shockwave_flash,
    application_x_tar,
    application_toolbook,
    application_x_tcl,
    text_x_script_tcl,
    text_x_script_tcsh,
    application_x_tex,
    application_x_texinfo,
    application_plain,
    application_gnutar,
    image_tiff,
    image_x_tiff,
    audio_tsp_audio,
    application_dsptype,
    audio_tsplayer,
    text_tab_separated_values,
    text_x_uil,
    text_uri_list,
    application_i_deas,
    application_x_ustar,
    multipart_x_ustar,
    text_x_uuencode,
    application_x_cdlink,
    text_x_vcalendar,
    application_vda,
    video_vdo,
    application_groupwise,
    video_vivo,
    video_vnd_vivo,
    application_vocaltec_media_desc,
    application_vocaltec_media_file,
    audio_voc,
    audio_x_voc,
    video_vosaic,
    audio_voxware,
    audio_x_twinvq_plugin,
    audio_x_twinvq,
    application_x_vrml,
    model_vrml,
    x_world_x_vrml,
    x_world_x_vrt,
    application_x_visio,
    application_wordperfect6_0,
    application_wordperfect6_1,
    audio_wav,
    audio_x_wav,
    application_x_qpro,
    image_vnd_wap_wbmp,
    application_vnd_xara,
    application_x_123,
    windows_metafile,
    text_vnd_wap_wml,
    application_vnd_wap_wmlc,
    text_vnd_wap_wmlscript,
    application_vnd_wap_wmlscriptc,
    application_wordperfect,
    application_x_wpwin,
    application_x_lotus,
    application_mswrite,
    application_x_wri,
    text_scriplet,
    application_x_wintalk,
    image_x_xbitmap,
    image_x_xbm,
    image_xbm,
    video_x_amt_demorun,
    xgl_drawing,
    image_vnd_xiff,
    application_excel,
    application_x_excel,
    application_x_msexcel,
    application_vnd_ms_excel,
    audio_xm,
    application_xml,
    text_xml,
    xgl_movie,
    application_x_vnd_ls_xpix,
    image_xpm,
    video_x_amt_showrun,
    image_x_xwd,
    image_x_xwindowdump,
    application_x_compress,
    application_x_zip_compressed,
    application_zip,
    multipart_x_zip,
    text_x_script_zsh
};

auto ls(std::string dir, std::vector<std::string>& result, bool recursive) -> int;
auto mkdir_recursive(std::string const& _name) -> bool;

auto copy_path(std::string const& _from, std::string const& _to) -> bool;
auto move_path(std::string const& _from, std::string const& _to) -> bool;
auto load_path(std::string const& _in, std::string& _out) -> bool;
auto load_path(std::string const& _in, std::wstring& _out) -> bool;
auto dump_path(std::string const& _in, std::string& _content) -> bool;
auto dump_path(std::string const& _in, std::wstring& _content) -> bool;

auto get_mime(std::string const& _in) -> zpt::MIMEType;
auto path_exists(std::string const& _in) -> bool;
auto is_dir(std::string const& _path) -> bool;
auto file_exists(std::string const& _path) -> bool;
auto dirname(std::string const& _path) -> std::string;

auto globRegexp(std::string& dir,
                std::vector<std::string>& result,
                std::regex& pattern,
                short recursion = 0) -> int;
auto glob(std::string dir,
          std::vector<std::string>& result,
          std::string pattern,
          short recursion = 0) -> int;
} // namespace zpt
