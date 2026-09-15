
# What I did:

I stumbled apon a kernel panic after mounting QEMU drive.
The problem was in the linux-next's broken filesystem block device driver. 
The fix is in the patch file.

# The Setup:

```bash
git config --global --remove-section sendemail

git config --global sendemail.smtpserver smtp.gmail.com
git config --global sendemail.smtpserverport 587
git config --global sendemail.smtpencryption tls

git config --global sendemail.smtpuser "your.email@gmail.com"
git config --global sendemail.from "Your Real Name <your.email@gmail.com>
```
# Sending patch to the LKML maintainers:

```bash
git send-email \
  --to="axboe@kernel.dk, akpm@linux-foundation.org" \
  --cc="tz2294@columbia.edu, hch@lst.de, jack@suse.cz, linux-fsdevel@vger.kernel.org, linux-kernel@vger.kernel.org" \
  0001-fs-buffer-fix-NULL-deref-on-folio-less-bh-in-__bh_submit.patch
```

