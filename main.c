#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include "MagicPico.h"
#include "MagicBoard.h"
#include "htmlfile.h"


#define WIFI_AP_SSID "PicoWebServer"
#define WIFI_AP_PASS "pico12345"

#define HTTP_PORT 80
#define HTTPS_PORT 443

#define LED_BRIGHTNESS 10

typedef struct
{
    const char *ptr;
    size_t left;
} http_state_t;

static bool handle_request(const char *request_line)
{
    printf(request_line);
    const char *band_param = strstr(request_line, "band=");


    if (band_param)
    {
        band_param += 5;
        if (strncmp(band_param, "rainbow", 7) == 0)
        {
            stop_animations(); 
            start_rainbow_animation(0xFF);
            broadcast_packet(taste_rainbow, taste_rainbow_len);
        }
        else if (strncmp(band_param, "circle", 6) == 0)
        {
            stop_animations(); 
            start_circle_animation(0xBF);
            broadcast_packet(circle, circle_len);
        }
        else if (strncmp(band_param, "connect", 7) == 0)
        {
            magicpico_wake();
        }
        else if (strncmp(band_param, "clear", 5) == 0)
        {
            stop_animations();
            magicpico_clear();
        }
        else if (strncmp(band_param, "pride", 5) == 0)
        {
            stop_animations();
            magicpico_clear();
            
            layers(urgb_u32(255, 0, 0, LED_BRIGHTNESS), 7);
            layers(urgb_u32(255, 165, 0, LED_BRIGHTNESS), 6);
            layers(urgb_u32(255, 255, 0, LED_BRIGHTNESS), 5);
            layers(urgb_u32(0, 128, 0, LED_BRIGHTNESS), 4);
            layers(urgb_u32(0, 0, 255, LED_BRIGHTNESS), 3);
            layers(urgb_u32(75, 0, 130, LED_BRIGHTNESS), 2);
            layers(urgb_u32(238, 130, 238, LED_BRIGHTNESS), 1);
            commit_pattern();
            broadcast_packet(taste_rainbow, taste_rainbow_len);
        }
        else if (strncmp(band_param, "bisexual", 8) == 0)
        {
            uint32_t colours[] = {urgb_u32(214, 2, 112,LED_BRIGHTNESS),urgb_u32(177, 10, 217,LED_BRIGHTNESS),urgb_u32(0, 56, 168,LED_BRIGHTNESS)};

            stop_animations();
            magicpico_clear();
            layers(colours[0], 7);
            layers(colours[0], 6);
            layers(colours[1], 5);
            layers(colours[1], 4);
            layers(colours[1], 3);
            layers(colours[2], 2);
            layers(colours[2], 1);
            commit_pattern();
            fivecolour(COLOUR_PURPLE, COLOUR_MIDNIGHT_BLUE, COLOUR_MIDNIGHT_BLUE, COLOUR_PINK, COLOUR_PINK, VIB_NONE);
        }
        else if (strncmp(band_param, "transgender", 11) == 0)
        {
            uint32_t colours[] = {urgb_u32(51, 106, 250, LED_BRIGHTNESS),urgb_u32(214, 2, 112, LED_BRIGHTNESS),urgb_u32(255,255,255, LED_BRIGHTNESS)};
            stop_animations();
            magicpico_clear();
            layers(colours[0], 7);
            layers(colours[0], 6);
            layers(colours[1], 5);
            layers(colours[2], 4);
            layers(colours[1], 3);
            layers(colours[0], 2);
            layers(colours[0], 1);
            commit_pattern();
            fivecolour(COLOUR_WHITE, COLOUR_PINK, COLOUR_BLUE, COLOUR_BLUE, COLOUR_PINK, VIB_NONE);
        }

        return true;
    }


    char *flicker_centre_param = strstr(request_line, "flickercentre=");
    char *flicker_topright_param = strstr(request_line, "flickertopright=");
    char *flicker_bottomright_param = strstr(request_line, "flickerbottomright=");
    char *flicker_topleft_param = strstr(request_line, "flickertopleft=");
    char *flicker_bottomleft_param = strstr(request_line, "flickerbottomleft=");
    char *flicker_vib_param = strstr(request_line, "flickervib=");
    char *flicker_speed_param = strstr(request_line, "flickerspeed=");
    if (flicker_centre_param && flicker_topright_param && flicker_bottomright_param && flicker_topleft_param && flicker_bottomleft_param && flicker_vib_param && flicker_speed_param) {
        uint8_t centre = (uint8_t)strtoul(flicker_centre_param + 14, NULL, 0);
        uint8_t top_right = (uint8_t)strtoul(flicker_topright_param + 17 , NULL, 0);
        uint8_t bottom_right = (uint8_t)strtoul(flicker_bottomright_param + 20, NULL, 0);
        uint8_t top_left = (uint8_t)strtoul(flicker_topleft_param + 16, NULL, 0);
        uint8_t bottom_left = (uint8_t)strtoul(flicker_bottomleft_param + 19, NULL, 0);
        uint8_t vibration = (uint8_t)strtoul(flicker_vib_param + 12, NULL, 0);
        uint8_t speed = (uint8_t)strtoul(flicker_speed_param + 13, NULL, 0);

        stop_animations(); 
        start_flicker_animation(speed, centre, top_right, bottom_right, top_left, bottom_left);
        five_slot_animation(centre, top_right, bottom_right, top_left, bottom_left, speed, vibration);
        return true;
    }


    const char *colour_param = strstr(request_line, "colour=");
    const char *vib_param = strstr(request_line, "vib=");
    const char *mask_param = strstr(request_line, "mask=");

    if (colour_param && vib_param && mask_param)
    {
        uint8_t colour =
            (uint8_t)strtoul(colour_param + 7, NULL, 0);
        uint8_t vibration =
            (uint8_t)strtoul(vib_param + 4, NULL, 0);
        uint8_t mask =
            (uint8_t)strtoul(mask_param + 5, NULL, 0);

        stop_animations(); 
        colour_fill(colour, mask);
        send_colour(
            colour,
            vibration,
            mask);

        return true;
    }

    const char *centre_param =
        strstr(request_line, "centre=");

    const char *top_right_param =
        strstr(request_line, "topright=");

    const char *bottom_right_param =
        strstr(request_line, "bottomright=");

    const char *top_left_param =
        strstr(request_line, "topleft=");

    const char *bottom_left_param =
        strstr(request_line, "bottomleft=");

    const char *multi_vib_param =
        strstr(request_line, "vib=");
    if (centre_param && top_right_param && bottom_right_param && top_left_param && bottom_left_param && multi_vib_param)
    {
         uint8_t centre =
        (uint8_t)strtoul(centre_param + 7, NULL, 0);

    uint8_t top_right =
        (uint8_t)strtoul(top_right_param + 9, NULL, 0);

    uint8_t bottom_right =
        (uint8_t)strtoul(bottom_right_param + 12, NULL, 0);

    uint8_t top_left =
        (uint8_t)strtoul(top_left_param + 8, NULL, 0);

    uint8_t bottom_left =
        (uint8_t)strtoul(bottom_left_param + 11, NULL, 0);

    uint8_t vibration =
        (uint8_t)strtoul(multi_vib_param + 4, NULL, 0);
        stop_animations(); 
        fivecolour(centre, top_right, bottom_right, top_left, bottom_left, vibration);
        section_fill(centre, top_right, bottom_right, top_left, bottom_left);
        return true;
    }

    const char *dual_outer_param = strstr(request_line, "dualouter=");
    const char *dual_centre_param = strstr(request_line, "dualcentre=");
    const char *dual_vib_param = strstr(request_line, "dualvib=");

    if (dual_centre_param && dual_vib_param && dual_outer_param)
    {
        uint8_t centre =
            (uint8_t)strtoul(dual_centre_param + 11, NULL, 0);
        uint8_t vibration =
            (uint8_t)strtoul(dual_vib_param + 8, NULL, 0);
        uint8_t outer = (uint8_t)strtoul(dual_outer_param + 10, NULL, 0);

        if (outer == 0x1F) {
            uint8_t valid_colours[] = {
                0x00, 0x01, 0x02, 0x03, 0x05, 0x06, 0x08, 
                0x0F, 0x10, 0x12, 0x13, 0x14, 0x15, 0x19, 
                0x1A, 0x1B
            };

            outer = valid_colours[rand() % (sizeof(valid_colours) / sizeof(valid_colours[0]))];
        }
        stop_animations();
        section_fill(centre, outer, outer, outer, outer);
        dualcolour(centre, outer, vibration);

        return true;
    }

    char *crossfade_colour_a_param = strstr(request_line, "crossfadecoloura=");
    char *crossfade_colour_b_param = strstr(request_line, "crossfadecolourb=");
    char *crossfade_vib_param = strstr(request_line, "crossfadevib=");
    char *crossfade_speed_param = strstr(request_line, "crossfadespeed=");
    if (crossfade_colour_a_param && crossfade_colour_b_param && crossfade_vib_param && crossfade_speed_param)
    {
        uint8_t colour_a = (uint8_t)strtoul(crossfade_colour_a_param + 17, NULL, 0);
        uint8_t colour_b = (uint8_t)strtoul(crossfade_colour_b_param + 17, NULL, 0);
        uint8_t vibration = (uint8_t)strtoul(crossfade_vib_param + 13, NULL, 0);
        uint8_t speed = (uint8_t)strtoul(crossfade_speed_param + 15, NULL, 0);

        stop_animations();
        start_crossfade_animation(colour_a, colour_b, speed);
        crossfade(colour_a, colour_b, vibration, speed);

        return true;
    }

    char *red_param = strstr(request_line, "red=");
    char *green_param = strstr(request_line, "green=");
    char *blue_param = strstr(request_line, "blue=");
    char *custom_vib_param = strstr(request_line, "customvib=");

if (red_param && green_param && blue_param && custom_vib_param) {
    uint8_t red = (uint8_t)strtoul(red_param + 4, NULL, 0);
    uint8_t green = (uint8_t)strtoul(green_param + 6, NULL, 0);
    uint8_t blue = (uint8_t)strtoul(blue_param + 5, NULL, 0);
    uint8_t vibration = (uint8_t)strtoul(custom_vib_param + 10, NULL, 0);

    stop_animations();
    custom_colour_fill(red, green, blue, MASK_ALL);
    customcolour(red, green, blue, vibration, false, false, false);
    
    return true;
}

    char *cornera = strstr(request_line, "cornerA=");
    char *cornerb = strstr(request_line, "cornerB=");
    char *cornerspeed = strstr(request_line, "cornerspeed=");
    char *corner_vib_param = strstr(request_line, "cornervib=");
    char *cornercentre = strstr(request_line, "cornercentre=");

if (cornera && cornerb && cornerspeed && corner_vib_param && cornercentre) {
    uint8_t corner_a = (uint8_t)strtoul(cornera + 8, NULL, 0);
    uint8_t corner_b = (uint8_t)strtoul(cornerb + 8, NULL, 0);
    uint8_t speed = (uint8_t)strtoul(cornerspeed + 12, NULL, 0);
    uint8_t vibration = (uint8_t)strtoul(corner_vib_param + 10, NULL, 0);
    uint8_t centre = (uint8_t)strtoul(cornercentre + 13, NULL, 0);

    stop_animations();
    start_corners_animation(speed, centre, corner_a, corner_b);
    corners_alternate(centre, corner_a, corner_b, corner_a, corner_b, speed, vibration);
    return true;
}

    char *diagonal1 = strstr(request_line, "diagonal1=");
    char *diagonal2 = strstr(request_line, "diagonal2=");
    char *diagonal3 = strstr(request_line, "diagonal3=");
    char *diagonal4 = strstr(request_line, "diagonal4=");
    char *diagonal5 = strstr(request_line, "diagonal5=");
    char *diagonal6 = strstr(request_line, "diagonal6=");
    char *diagonal7 = strstr(request_line, "diagonal7=");
    char *diagonal_orientation = strstr(request_line, "orientation=");
    char *diagonalVib = strstr(request_line, "diagonalVib=");

    if (diagonal1 && diagonal2 && diagonal3 && diagonal4 && diagonal5 && diagonal6 && diagonal7 && diagonalVib && diagonal_orientation) {
        uint8_t d1 = (uint8_t)strtoul(diagonal1 + 10, NULL, 0);
        uint8_t d2 = (uint8_t)strtoul(diagonal2 + 10, NULL, 0);
        uint8_t d3 = (uint8_t)strtoul(diagonal3 + 10, NULL, 0);
        uint8_t d4 = (uint8_t)strtoul(diagonal4 + 10, NULL, 0);
        uint8_t d5 = (uint8_t)strtoul(diagonal5 + 10, NULL, 0);
        uint8_t d6 = (uint8_t)strtoul(diagonal6 + 10, NULL, 0);
        uint8_t d7 = (uint8_t)strtoul(diagonal7 + 10, NULL, 0);
        uint8_t orientation = (uint8_t)strtoul(diagonal_orientation + 12, NULL, 0);
        uint8_t vib = (uint8_t)strtoul(diagonalVib + 12, NULL, 0);
        bool orientation_toggle = (orientation == 1) ? true : false;
        stop_animations();
        diagonalsband(d1, d2, d3, d4, d5, d6, d7, orientation_toggle);
        fivecolour(d1, d2, d3, d4, d5, vib);
        
        return true;
    }

    char *layer1 = strstr(request_line, "layer1=");
    char *layer2 = strstr(request_line, "layer2=");
    char *layer3 = strstr(request_line, "layer3=");
    char *layer4 = strstr(request_line, "layer4=");
    char *layer5 = strstr(request_line, "layer5=");
    char *layer6 = strstr(request_line, "layer6=");
    char *layer7 = strstr(request_line, "layer7=");
    char *layerVib = strstr(request_line, "layerVib=");

    if (layer1 && layer2 && layer3 && layer4 && layer5 && layer6 && layer7 && layerVib) {
        uint8_t l1 = (uint8_t)strtoul(layer1 + 7, NULL, 0);
        uint8_t l2 = (uint8_t)strtoul(layer2 + 7, NULL, 0);
        uint8_t l3 = (uint8_t)strtoul(layer3 + 7, NULL, 0);
        uint8_t l4 = (uint8_t)strtoul(layer4 + 7, NULL, 0);
        uint8_t l5 = (uint8_t)strtoul(layer5 + 7, NULL, 0);
        uint8_t l6 = (uint8_t)strtoul(layer6 + 7, NULL, 0);
        uint8_t l7 = (uint8_t)strtoul(layer7 + 7, NULL, 0);
        uint8_t vib = (uint8_t)strtoul(layerVib + 9, NULL, 0);
        stop_animations();
        layersband(l1, l2, l3, l4, l5, l6, l7);
        fivecolour(l1, l2, l3, l4, l5, vib);
        
        
        return true;
    }

    return false;
}


static void http_close_connection(struct tcp_pcb *tpcb){
    if (tpcb != NULL){
        http_state_t *hs =(http_state_t *)tpcb->callback_arg;
        if (hs){
            free(hs);}
        tcp_arg(tpcb, NULL);
        tcp_sent(tpcb, NULL);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_close(tpcb);
    }
}

static void http_err_callback(void *arg, err_t err){
    (void)err;
    http_state_t *hs = (http_state_t *)arg;
    if (hs){
        free(hs);
    }
}

static void http_send_data(struct tcp_pcb *tpcb, http_state_t *hs){

    while (hs->left > 0 && tcp_sndbuf(tpcb) > 0){
        uint16_t available = tcp_sndbuf(tpcb);
        uint16_t chunk_size = (available > 512) ? 512 : available;

        if (chunk_size > hs->left){chunk_size = (uint16_t)hs->left;}

        err_t err = tcp_write(tpcb, hs->ptr, chunk_size,0);

        if (err != ERR_OK){
            break;
        }

        hs->ptr += chunk_size;
        hs->left -= chunk_size;
    }
    tcp_output(tpcb);
}

static err_t http_sent_callback(void *arg, struct tcp_pcb *tpcb, uint16_t len){
    (void)len;
    http_state_t *hs = (http_state_t *)arg;

    if (hs && hs->left > 0){
        http_send_data(tpcb, hs);
    }
    else{
        http_close_connection(tpcb);
    }

    return ERR_OK;
}

static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){
    (void)arg;
    if (p == NULL){
        http_close_connection(tpcb);
        return ERR_OK;
    }

    tcp_recved(tpcb, p->tot_len);

    if (err != ERR_OK){
        pbuf_free(p);
        http_close_connection(tpcb);
        return err;
    }

    char request_buffer[512];
    size_t copy_len = p->tot_len < sizeof(request_buffer) - 1 ? p->tot_len: sizeof(request_buffer) - 1;
    pbuf_copy_partial(p, request_buffer, copy_len, 0);
    request_buffer[copy_len] ='\0';
    pbuf_free(p);


    if (strstr(request_buffer,"hotspot-detect.html") || strstr(request_buffer,"generate_204") || strstr(request_buffer, "connecttest.txt" )){
        const char *apple_ok =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 51\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<HTML><HEAD><TITLE>Connection Successful</TITLE></HEAD>"
            "<BODY>Successfully connected, go to magic.pico to control the magic</BODY></HTML>";

        tcp_write(tpcb, apple_ok, strlen(apple_ok),TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        http_close_connection(tpcb);
        return ERR_OK;
    }


    bool is_api = handle_request(request_buffer);
    if (is_api){
        const char *api_ok =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: 8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"ok\":1}";

        tcp_write(tpcb, api_ok, strlen(api_ok), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        http_close_connection(tpcb);
        return ERR_OK;
    }

    http_state_t *hs =(http_state_t *)malloc(sizeof(http_state_t));

    if (!hs){
        http_close_connection(tpcb);
        return ERR_MEM;
    }

    hs->ptr = HTTP_RESPONSE_BODY_TEMPLATE;
    hs->left = strlen(HTTP_RESPONSE_BODY_TEMPLATE);

    tcp_arg(tpcb, hs);

    tcp_err(tpcb, http_err_callback);
    tcp_sent(tpcb, http_sent_callback);

    char headers[128];

    int header_len = snprintf(headers,sizeof(headers),"HTTP/1.1 200 OK\r\n" 
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n", hs->left);

    tcp_write(tpcb, headers, header_len, TCP_WRITE_FLAG_COPY);
    http_send_data(tpcb, hs);
    return ERR_OK;
}

static err_t https_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err){
    (void)arg;
    (void)err;

    if (newpcb != NULL){
        tcp_abort(newpcb);
    }

    return ERR_ABRT;
}

static err_t http_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err){
    (void)arg;
    if (err != ERR_OK || newpcb == NULL){return ERR_VAL;}
    tcp_arg(newpcb, NULL);
    tcp_recv(newpcb, http_recv_callback);
    tcp_err(newpcb, http_err_callback);

    return ERR_OK;
}


static bool start_http_servers(void){
    struct tcp_pcb *pcb_80 = tcp_new();
    if (!pcb_80 || tcp_bind(pcb_80, IP_ADDR_ANY, HTTP_PORT) != ERR_OK){
        return false;
    }

    struct tcp_pcb *listen_80 = tcp_listen(pcb_80);
    if (!listen_80){
        return false;
    }

    tcp_accept(listen_80, http_accept_callback);

    struct tcp_pcb *pcb_443 = tcp_new();
    if (pcb_443 && tcp_bind(pcb_443, IP_ADDR_ANY, HTTPS_PORT) == ERR_OK){
        struct tcp_pcb *listen_443 = tcp_listen(pcb_443);
        if (listen_443){
            tcp_accept(listen_443, https_accept_callback);
        }
    }

    return true;
}


int main(void){

    stdio_init_all();
    sleep_ms(3000);

    if (cyw43_arch_init()){
        printf("cyw43_arch_init failed\n");
        return -1;
    }

    cyw43_arch_enable_ap_mode(WIFI_AP_SSID, WIFI_AP_PASS, CYW43_AUTH_WPA2_AES_PSK);



    ip4_addr_t gateway_ip;
    ip4_addr_t netmask;

    IP4_ADDR(&gateway_ip, 192, 168, 4, 1);

    IP4_ADDR(&netmask, 255, 255, 255, 0);

    netif_set_addr(&cyw43_state.netif[CYW43_ITF_AP], &gateway_ip, &netmask,&gateway_ip);

    netif_set_up(&cyw43_state.netif[CYW43_ITF_AP]);

    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &cyw43_state.netif[CYW43_ITF_AP], &gateway_ip, &netmask);

    dns_server_t dns_server;
    dns_server_init(&dns_server, &cyw43_state.netif[CYW43_ITF_AP],&gateway_ip);

    if (!start_http_servers())
    {
        printf("Failed to start HTTP servers\n");
        return -1;
    }

    printf("Initialising MagicPico BLE advertiser...\n");
    magicpico_init();

    initleds(16, 31, LED_BRIGHTNESS);

    while (!magicpico_is_ready()){
        cyw43_arch_poll();
        sleep_ms(1);
    }


    while (true){
        cyw43_arch_poll();
        //magicpico_update();
        sleep_ms(1);
    }
    
}
