cp ./app/build/outputs/apk/release/app-release-unsigned.apk __tmp__.apk
/home/trentfromcanmore/android/sdk/build-tools/29.0.3/zipalign -c -v 4 __tmp__.apk
#/home/trentfromcanmore/android/sdk/build-tools/29.0.3/zipalign -p -v 4 __tmp__.apk __tmp__-aligned.apk
/home/trentfromcanmore/android/sdk/build-tools/29.0.3/apksigner sign --min-sdk-version 23 --ks-key-alias my-alias --ks ../../new-release-key.jks --out __tmp__-signed.apk __tmp__.apk
/home/trentfromcanmore/android/sdk/build-tools/29.0.3/apksigner verify __tmp__-signed.apk
mv __tmp__-signed.apk $1
#rm __tmp__-aligned.apk
rm __tmp__.apk
