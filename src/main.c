/* main.c
 *
 * Copyright (C) 2017 Sergiusz Urbaniak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <glib.h>
#include <gst/gst.h>
#include <alsa/asoundlib.h>

gboolean
device_monitor_bus_handler (GstBus *bus,
                            GstMessage *m,
                            gpointer user_data)
{
  g_debug ("device monitor bus message type %d", m->type);

  const GstStructure *s = gst_message_get_structure (m);
  if (s == NULL)
            return TRUE;

  gchar *str = gst_structure_to_string (s);
  g_debug ("device monitor struct %s", str);
  g_free (str);

  return TRUE;
}

unsigned int
log2ui (unsigned int val) {
    if (val == 0)
      return UINT_MAX;

    if (val == 1)
      return 0;

    unsigned int ret = 0;
    while (val > 1) {
        val >>= 1;
        ret++;
    }

    return ret;
}

gboolean
debug_structure (GQuark field_id,
                 const GValue *value,
                 gpointer user_data)
{
  gchar *str = g_strdup_value_contents(value);
  g_debug ("field %d value %s", field_id, str);
  g_free (str);
  return TRUE;
}

void
debug_structure_fields (const GstStructure*s)
{
  /* print msg structure names and type */
  for (int i = 0; i < gst_structure_n_fields (s); i++) {
    const gchar *name = gst_structure_nth_field_name (s, i);
    GType type = gst_structure_get_field_type (s, name);
    const GValue *value = gst_structure_get_value (s, name);
    gchar *values = g_strdup_value_contents(value);

    g_debug ("%s[%s] = %s",
             name,
             g_type_name (type),
             values);

    g_free (values);
  }
}

void
debug_rms (const GstStructure *s)
{
  gint channels;
  GstClockTime endtime;
  gdouble rms_dB, peak_dB, decay_dB;
  gdouble rms;
  const GValue *array_val;
  const GValue *value;
  GValueArray *rms_arr, *peak_arr, *decay_arr;
  gint i;

  if (!gst_structure_get_clock_time (s, "endtime", &endtime))
    g_warning ("Could not parse endtime");

  /* the values are packed into GValueArrays with the value per channel */
  array_val = gst_structure_get_value (s, "rms");
  rms_arr = (GValueArray *) g_value_get_boxed (array_val);

  array_val = gst_structure_get_value (s, "peak");
  peak_arr = (GValueArray *) g_value_get_boxed (array_val);

  array_val = gst_structure_get_value (s, "decay");
  decay_arr = (GValueArray *) g_value_get_boxed (array_val);

  /* we can get the number of channels as the length of any of the value
   * arrays */
  channels = rms_arr->n_values;
  g_print ("endtime: %" GST_TIME_FORMAT ", channels: %d\n",
           GST_TIME_ARGS (endtime), channels);
  for (i = 0; i < channels; ++i) {

    g_print ("channel %d\n", i);
    value = rms_arr->values + i;
    rms_dB = g_value_get_double (value);

    value = peak_arr->values + i;
    peak_dB = g_value_get_double (value);

    value = decay_arr->values + i;
    decay_dB = g_value_get_double (value);

    g_print ("    RMS: %f dB, peak: %f dB, decay: %f dB\n",
             rms_dB, peak_dB, decay_dB);

    /*     converting from dB to normal gives us a value between 0.0 and 1.0 */
    rms = pow (10, rms_dB / 20);
    g_print ("    normalized rms value: %f\n", rms);
  }
}

gboolean
pipeline_bus_handler (GstBus *bus,
                      GstMessage *m,
                      gpointer user_data)
{
  g_debug ("pipeline message type %d", m->type);

  const GstStructure *s = gst_message_get_structure (m);
  if (s == NULL)
    return TRUE;

  debug_structure_fields (s);

  gchar *str = gst_structure_to_string (s);
  g_debug ("pipeline struct %s", str);
  g_free (str);

  const gchar *name = gst_structure_get_name (s);
  g_debug ("%s", name);

  if (m->type != GST_MESSAGE_ELEMENT)
    return TRUE;

  debug_rms (s);
  return TRUE;
}

void
device_list (void)
{
  snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;

  snd_ctl_t *handle;
  int card, err, dev, idx;

  snd_ctl_card_info_t *info;
  snd_ctl_card_info_alloca (&info);

  snd_pcm_info_t *pcminfo;
  snd_pcm_info_alloca (&pcminfo);

  card = -1;
  if (snd_card_next (&card) < 0 || card < 0) {
    g_error ("no soundcards found...");
    return;
  }

  while (card >= 0) {
    GString *name = g_string_new (NULL);
    g_string_printf(name, "hw:%d", card);

    if ((err = snd_ctl_open (&handle, name->str, 0)) < 0) {
      g_error ("control open (%i): %s", card, snd_strerror (err));
      goto next_card;
    }

    if ((err = snd_ctl_card_info (handle, info)) < 0) {
      g_error ("control hardware info (%i): %s", card, snd_strerror (err));
      snd_ctl_close (handle);
      goto next_card;
    }

    dev = -1;

    while (1) {
      unsigned int count;

      if (snd_ctl_pcm_next_device (handle, &dev)<0)
        g_error("snd_ctl_pcm_next_device");

      if (dev < 0)
        break;

      snd_pcm_info_set_device (pcminfo, dev);
      snd_pcm_info_set_subdevice (pcminfo, 0);
      snd_pcm_info_set_stream (pcminfo, stream);

      if ((err = snd_ctl_pcm_info (handle, pcminfo)) < 0) {
        if (err != -ENOENT)
          g_error ("control digital audio info (%i): %s",
                   card, snd_strerror(err));

        continue;
      }

      printf("card %i: %s [%s], device %i: %s [%s]\n",
             card, snd_ctl_card_info_get_id (info), snd_ctl_card_info_get_name (info),
             dev,
             snd_pcm_info_get_id(pcminfo),
             snd_pcm_info_get_name(pcminfo));

      count = snd_pcm_info_get_subdevices_count (pcminfo);

      printf("  Subdevices: %i/%i\n",
              snd_pcm_info_get_subdevices_avail (pcminfo), count);

      for (idx = 0; idx < (int)count; idx++) {
        snd_pcm_info_set_subdevice (pcminfo, idx);

        if ((err = snd_ctl_pcm_info (handle, pcminfo)) < 0) {
          g_error ("control digital audio playback info (%i): %s",
                   card, snd_strerror (err));
        } else {
          printf("  Subdevice #%i: %s\n",
                 idx, snd_pcm_info_get_subdevice_name(pcminfo));
        }
      }
    }
    snd_ctl_close (handle);

  next_card:
    g_string_free (name, TRUE);
    if (snd_card_next (&card) < 0) {
      g_error ("snd_card_next");
      break;
    }
  }
}

int
main (int argc, char *argv[])
{
  gst_init (&argc, &argv);
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  GstElement *bin = gst_pipeline_new ("bin");
  g_assert (bin);
  GstElement *src = gst_element_factory_make ("autoaudiosrc", "src");
  g_assert (src);
  GstElement *audioconvert = gst_element_factory_make ("audioconvert", NULL);
  g_assert (audioconvert);
  GstElement *level = gst_element_factory_make ("level", NULL);
  g_assert (level);
  GstElement *audioresample = gst_element_factory_make ("audioresample", NULL);
  g_assert (audioresample);
  GstElement *vorbisenc = gst_element_factory_make ("vorbisenc", NULL);
  g_assert (vorbisenc);
  GstElement *oggmux = gst_element_factory_make ("oggmux", NULL);
  g_assert (oggmux);
  GstElement *filesink = gst_element_factory_make ("filesink", NULL);
  g_assert (filesink);

  g_object_set (G_OBJECT (filesink),
                "location", "test.ogg",
                NULL);

  g_object_set (G_OBJECT (level),
                "post-messages", TRUE,
                NULL);

  g_object_set (G_OBJECT (level),
                "interval", 100000000,
                NULL);

  gst_bin_add_many (GST_BIN (bin),
                    src,
                    audioconvert,
                    level,
                    audioresample,
                    vorbisenc,
                    oggmux,
                    filesink,
                    NULL);

  GstCaps *caps = gst_caps_from_string ("audio/x-raw,channels=2");

  g_assert (gst_element_link (src, audioconvert) &&
            gst_element_link_filtered (audioconvert, level, caps) &&
            gst_element_link (level, audioresample) &&
            gst_element_link (audioresample, vorbisenc) &&
            gst_element_link (vorbisenc, oggmux) &&
            gst_element_link (oggmux, filesink));

  GstBus *bus = gst_element_get_bus (bin);
  gst_bus_add_watch (bus, pipeline_bus_handler, NULL);
  gst_object_unref (bus);

  GstDeviceMonitor *mon = gst_device_monitor_new ();
  GstBus *monBus = gst_device_monitor_get_bus (mon);
  gst_bus_add_watch (monBus, device_monitor_bus_handler, NULL);
  if (!gst_device_monitor_start (mon))
    {
      g_info("no device monitor support\n");
    }

  gst_element_set_state (bin, GST_STATE_PLAYING);

  if (gst_element_get_state (bin, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE)
    {
      g_error ("Failed to go into PLAYING state");
      goto failure;
    }

  device_list ();

  g_main_loop_run (loop);

  gst_object_unref (monBus);
  gst_object_unref (mon);

 failure:
  gst_object_unref (bin);
}
