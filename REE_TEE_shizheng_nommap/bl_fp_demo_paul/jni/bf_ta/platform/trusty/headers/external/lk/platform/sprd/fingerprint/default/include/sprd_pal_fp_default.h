/*
* Copyright (c) 2016, Spreadtrum Communications.
*
* The above copyright notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _SPRD_PAL_CHIPONE_H_
#define _SPRD_PAL_CHIPONE_H_

#ifdef __cplusplus
extern "C"
{
#endif


enum fp_default_cmd_info{
  SPI_WRITE_AND_READ = 0,
  CMD_MAX
};



struct WRITE_THEN_READ_STR{
    uint32_t    max_speed_hz;
    uint8_t     chip_select;
    uint8_t     mode;
    uint8_t     bits_per_word;
    uint8_t     number;
    uint8_t*    rxbuf;
    uint8_t*    txbuf;
    uint32_t    len;
    uint8_t     debug;
};




#ifdef __cplusplus
}
#endif

#endif
