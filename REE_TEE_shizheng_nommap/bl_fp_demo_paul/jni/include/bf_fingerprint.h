#ifndef __BF_FINGERPRINT_H__
#define __BF_FINGERPRINT_H__
int bf_fingerprint_close(hw_device_t *device);
uint64_t bf_fingerprint_pre_enroll(struct fingerprint_device __unused *device);
int bf_fingerprint_enroll(struct fingerprint_device __unused *device,
                              const hw_auth_token_t __unused *hat,
                              uint32_t __unused gid,
                              uint32_t __unused timeout_sec);
int bf_fingerprint_post_enroll(struct fingerprint_device __unused *device);
uint64_t bf_get_authenticator_id(struct fingerprint_device __unused *device);
int bf_fingerprint_cancel(struct fingerprint_device __unused *device);
int bf_fingerprint_remove(struct fingerprint_device __unused *device,
                              uint32_t __unused gid, uint32_t __unused fid);
int bf_fingerprint_set_active_group(struct fingerprint_device __unused *device,
                                        uint32_t __unused gid, const char __unused *store_path);
int bf_fingerprint_authenticate(struct fingerprint_device __unused *device,
                                    uint64_t __unused operation_id, __unused uint32_t gid);
int bf_set_notify(struct fingerprint_device *device,
                               fingerprint_notify_t notify);
int bf_fingerprint_enumerate(struct fingerprint_device *device);
int bf_fingerprint_open(const hw_module_t* module, const char __unused *id,
                            hw_device_t** device);
#endif
