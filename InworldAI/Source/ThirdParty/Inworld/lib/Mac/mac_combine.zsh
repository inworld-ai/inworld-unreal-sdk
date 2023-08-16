
for i in InworldNdk absl_base absl_malloc_internal absl_raw_logging_internal absl_spinlock_wait absl_throw_delegate absl_bad_optional_access absl_cord absl_str_format_internal absl_strings absl_strings_internal absl_symbolize absl_stacktrace absl_graphcycles_internal absl_synchronization absl_int128 absl_status absl_statusor absl_time absl_time_zone address_sorting gpr grpc grpc++ cares protobuf re2 upb
do
    echo Bundling library: $i
    lipo arm64/lib$i.a x86_64/lib$i.a -create -output lib$i.a
done

echo Bundling dylib
lipo arm64/libwebrtc_aec_plugin.dylib x86_64/libwebrtc_aec_plugin.dylib -create -output libwebrtc_aec_plugin.dylib
