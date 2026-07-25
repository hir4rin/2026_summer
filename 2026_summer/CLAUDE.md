# 2026_summer 実装メモ

学校/家をまたいで作業を引き継ぐためのメモ。Claude Codeは起動時にこのファイルを
自動で読み込む。作業を区切るタイミングで各欄を更新すること。

## 現在取り組んでいるタスク

CollisionManagerに Trigger（接触開始）／Press（接触中）／Exit（接触終了）を
検知する機能を追加する。

- 作業ブランチ: `claude/collision-manager-press-exit-trigger-227891`
  （worktree: `2026_summer/.claude/worktrees/collision-manager-press-exit-trigger-227891/`）
- 2026-07-25時点でmainから未着手（コミット差分なし）

## 設計方針・制約

- 既存の `CollisionInfo` / `Collider` の構造はできるだけ変更しない

## TODO

- [ ] Collider に状態（未接触・接触中など）を持たせる設計から着手する

## 作業ログ

- 2026-07-25: CollisionManagerのTrigger/Press/Exit機能追加に着手予定として整理。
  worktree `claude/collision-manager-press-exit-trigger-227891` が用意済み（未着手）だと判明。
- 2026-07-24: エフェクトの再生位置がカメラ位置起因でバグる件、
  レンダリングを分けている影響でエフェクトが全部描画されてしまう件、いずれも解決済み。
