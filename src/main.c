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

#include <gst/gst.h>

int
main(int argc, char *argv[])
{
  gst_init (&argc, &argv);
  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  GstElement *bin = gst_pipeline_new ("bin");
  GstElement *src = gst_element_factory_make ("autoaudiosrc", "src");
  GstElement *audioconvert = gst_element_factory_make ("audioconvert", NULL);
  GstElement *vorbisenc = gst_element_factory_make ("vorbisenc", NULL);
  GstElement *oggmux = gst_element_factory_make ("oggmux", NULL);
  GstElement *filesink = gst_element_factory_make ("filesink", NULL);

  g_object_set (G_OBJECT (filesink),
                "location", "test.ogg",
                NULL);

  gst_bin_add_many (GST_BIN (bin),
                    src,
                    audioconvert,
                    vorbisenc,
                    oggmux,
                    filesink,
                    NULL);

  if (!gst_element_link (src, audioconvert) ||
      !gst_element_link (audioconvert, vorbisenc) ||
      !gst_element_link (vorbisenc, oggmux) ||
      !gst_element_link (oggmux, filesink))
    {
      g_error("can't link elements\n");
      goto failure;
    }

  gst_element_set_state (bin, GST_STATE_PLAYING);

  if (gst_element_get_state (bin, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE)
    {
      g_error ("Failed to go into PLAYING state");
      goto failure;
    }

  g_main_loop_run (loop);

 failure:
  gst_object_unref (bin);
}
