
# Autopush Module Change Log

## [ToDo]

- Recrypt linkedin tokens and resource ids with the local encryption key before saving.
- Translate the language file.
- Test if the other methods weren't broken with this last change.
- Review tokens management.
- Release and push.

# [1.5.0] - 2024-07-02

- Updated support for changes in APIs (LinkedIn, Twitter and Telegram).
- Added support for pushing to telegram topics in SuperGroups.

## [1.4.1] - 2020-04-27

- Added POST headers recently required by Discord.

## [1.4.0] - 2020-04-20

- Added "Push to" button on single post actions.
- Added Spanish translation.
- Added Message pushing from the main menu (mods/admins only).
- Removed autopush controls on the quick post form
  (it conflicts with message pushing from the main menu).
- Added automated push per-user-level limits.
- Added automated push multiple per-category/fallback messages.

## [1.3.0] - 2020-04-07

- Improvements on the settings editor.
- Added post visibility validations.
- Added support for Telegram.
- Added support for automated push by other users.
- Other minor corrections.

## [1.2.1] - 2019-07-01

- Tweaked file title on twitter image uploads.

## [1.2.0] - 2019-07-01

- Fixed doc errors.
- Detached endpoints preload to the pre-rendering area for easing injection by other modules.
- Detached form and its styles and scripts from the post extender for easing injection by other modules.
- Added remote images download and upload for twitter.
- Added custom URL generic/callable pushing method.

## [1.1.0] - 2019-07-01

- Added custom message when sending as link.

## [1.0.0] - 2019-06-29

- Initial release.
